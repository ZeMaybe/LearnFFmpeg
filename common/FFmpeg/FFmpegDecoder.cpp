
#include "FFmpegDecoder.h"
#include "FFmpegHwDecoderHelper.h"

FFmpegDecoder::FFmpegDecoder(const char* filePath, int id, bool hwAcc)
    :Id(id)
    , hwAccel(hwAcc)
{
    packet = av_packet_alloc();
    videoFrame1 = av_frame_alloc();
    videoFrame2 = av_frame_alloc();

    loadFile(filePath, hwAccel);
}

FFmpegDecoder::~FFmpegDecoder()
{
    if (helper != nullptr)
        delete helper;

    resetCodec();
    clearInternalData();
}

#define check_false_log_return(v,l,t,f)  if(v == false){\
    av_log(nullptr,l,t);\
    f;\
    return v;\
}\

bool FFmpegDecoder::loadFile(const char* filePath, bool hwAcc)
{
    hwAccel = hwAcc;
    if (filePath == nullptr)
    {
        return false;
    }

    bool re = true;
    resetCodec();
    resetInternalData();

    re = (formatCtx = avformat_alloc_context()) != nullptr;
    check_false_log_return(re, AV_LOG_WARNING, "couldn't allocate AVFormatContext.\n", void);

    re = avformat_open_input(&formatCtx, filePath, nullptr, nullptr) == 0;
    check_false_log_return(re, AV_LOG_WARNING, "couldn't open input file.\n", resetCodec());

    re = avformat_find_stream_info(formatCtx, nullptr) >= 0;
    check_false_log_return(re, AV_LOG_WARNING, "couldn't find stream informations.\n", resetCodec());

    videoIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    re = (videoIndex >= 0) && (codec != nullptr);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't find video stream.\n", resetCodec());

    codecCtx = avcodec_alloc_context3(codec);
    re = (codecCtx != nullptr);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't allocate codec context.\n", resetCodec());

    re = (avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoIndex]->codecpar) >= 0);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't copy parameters to codec context.\n", resetCodec());

    if (hwAccel)
    {
        helper = new FFmpegHwDecoderHelper(this);
        helper->setupHwAcceleration("cuda");
    }

    re = (avcodec_open2(codecCtx, codec, nullptr) == 0);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't open codec context.\n", resetCodec());

    running = true;
    decoderThread = std::thread(FFmpegDecoder::decoderFunction, this);
    return re;
}

bool FFmpegDecoder::receiveVideoFrame(AVFrame* pFrame)
{
    bool re = false;
    if (pFrame != nullptr)
    {
        av_frame_unref(pFrame);
        if (frame2Ready)
        {
            {
                std::lock_guard g(video2Mutex);
                av_frame_move_ref(pFrame, videoFrame2);
                frame2Ready = false;
                re = true;
                //av_log(nullptr, AV_LOG_INFO, "take one!");
            }
        }
    }
    return re;
}

void FFmpegDecoder::setOutputFramePixelFormat(AVPixelFormat newFmt)
{
    //av_hwframe_transfer_data()
    // need to be done.
    //outputFramePixelFormat = newFmt;
    // need to add swscontext or other thing to convert pixel format.. 
}

#define check_false_log_break(v,l,t)  if(v == false){\
    av_log(nullptr,l,t);\
    break;\
}\

int FFmpegDecoder::getPixelWidth() const
{
    int re = 0;
    if (codecCtx != nullptr)
    {
        codecCtx->width;
    }
    return re;
}

int FFmpegDecoder::getPixelHeight() const
{
    int re = 0;
    if (codecCtx != nullptr)
    {
        re = codecCtx->height;
    }
    return re;
}

void FFmpegDecoder::getPixelSize(int* w, int* h) const
{
    if (w == nullptr || h == nullptr)
    {
        return;
    }

    if (codecCtx != nullptr)
    {
        *w = codecCtx->width;
        *h = codecCtx->height;
    }
}

double FFmpegDecoder::getFrameRate() const
{
    return av_q2d(codecCtx->framerate);
}

void FFmpegDecoder::resetCodec()
{
    if (decoderThread.joinable())
    {
        running = false;
        decoderThread.join();
    }

    if (codecCtx)
    {
        avcodec_free_context(&codecCtx);
    }

    if (formatCtx)
    {
        avformat_close_input(&formatCtx);
        avformat_free_context(formatCtx);
    }

    codec = nullptr;
    codecCtx = nullptr;
    formatCtx = nullptr;
    videoIndex = -1;
    outputFramePixelFormat = AV_PIX_FMT_NONE;
}

void FFmpegDecoder::resetInternalData()
{
    if (decoderThread.joinable())
    {
        running = false;
        decoderThread.join();
    }

    av_packet_unref(packet);
    av_frame_unref(videoFrame1);
    frame1Ready = false;
    {
        std::lock_guard g(video2Mutex);
        av_frame_unref(videoFrame2);
        frame2Ready = false;
    }
}

void FFmpegDecoder::clearInternalData()
{
    resetInternalData();
    av_packet_free(&packet);
    av_frame_free(&videoFrame1);
    av_frame_free(&videoFrame2);
}

void FFmpegDecoder::decoderFunction(FFmpegDecoder* d)
{
    if (d == nullptr) return;

    av_log(nullptr, AV_LOG_INFO, "decoder %d function start.\n", d->Id);
    int ret;
    while (d->running)
    {
        if (d->frame1Ready == false)
        {
            av_packet_unref(d->packet);
            ret = av_read_frame(d->formatCtx, d->packet);
            if (ret < 0)
            {
                d->sendPacketAndReceiveFrame(nullptr);
                d->running = false;
                break;
            }

            if (d->packet->stream_index == d->videoIndex)
            {
                ret = d->sendPacketAndReceiveFrame(d->packet);
                if (ret == -1)
                {
                    d->running = false;
                    break;
                }

                if (ret == 0)
                {
                    d->sendPacketAndReceiveFrame(nullptr);
                    d->running = false;
                    break;
                }
            }
        }
    }
    av_log(nullptr, AV_LOG_INFO, "decoder %d function stoped.\n", d->Id);
}

// return value : 0  -> end of file
//                1  -> send packet again
//               -1  -> error occured    
int FFmpegDecoder::sendPacketAndReceiveFrame(AVPacket* packet)
{
    int ret = avcodec_send_packet(codecCtx, packet);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_WARNING, "Failed to send packet.\n");
        return -1;
    }

    while (running)
    {
        if (frame1Ready == false)
        {
            ret = avcodec_receive_frame(codecCtx, videoFrame1);
            if (ret == AVERROR(EAGAIN))
            {
                return 1;
            }

            if (ret == AVERROR_EOF)
            {
                return 0;
            }

            if (ret < 0)
            {
                av_log(nullptr, AV_LOG_WARNING, "Failed to receive frame.\n");
                return -1;
            }

            frame1Ready = true;
            //if (Id == 2)
                //SDL_Log("%d : get one frame.", Id);
        }

        if (frame1Ready == true && frame2Ready == false)
        {
            std::lock_guard g(video2Mutex);
            std::swap(videoFrame1, videoFrame2);
            frame1Ready = false;
            frame2Ready = true;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return -1;
}
