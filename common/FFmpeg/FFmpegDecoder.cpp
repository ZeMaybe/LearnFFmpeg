
#include "FFmpegDecoder.h"

extern "C"
{
#include "SDL.h"
}

FFmpegDecoder::FFmpegDecoder(const char* filePath)
{
    packet = av_packet_alloc();
    videoFrame1 = av_frame_alloc();
    videoFrame2 = av_frame_alloc();

    loadFile(filePath);
}

FFmpegDecoder::~FFmpegDecoder()
{
    resetCodec();
    clearInternalData();
}

#define check_false_log_return(v,l,t,f)  if(v == false){\
    av_log(nullptr,l,t);\
    f;\
    return v;\
}\

bool FFmpegDecoder::loadFile(const char* filePath)
{
    if (filePath == nullptr)
    {
        return false;
    }

    bool re = true;
    resetCodec();
    resetInternalData();

    re = (formatCtx = avformat_alloc_context()) != nullptr;
    check_false_log_return(re, AV_LOG_WARNING, "couldn't allocate AVFormatContext.\n",void);

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
                std::lock_guard g(videoMutex);
                av_frame_move_ref(pFrame, videoFrame2);
                frame2Ready = false;
                re = true;
            }
        }
    }
    return re;
}

void FFmpegDecoder::setOutputFramePixelFormat(AVPixelFormat newFmt)
{
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

void FFmpegDecoder::decoderFunction(FFmpegDecoder* d)
{
    if (d == nullptr)
    {
        return;
    }

    int ret;
    do
    {
        if (d->frame1Ready == true && d->frame2Ready == false)
        {
            std::lock_guard g(d->videoMutex);
            std::swap(d->videoFrame1, d->videoFrame2);
            d->frame1Ready = false;
            d->frame2Ready = true;
        }

        if (d->frame1Ready == false)
        {
            av_packet_unref(d->packet);
            ret = av_read_frame(d->formatCtx, d->packet);
            d->running = (ret == 0);
            check_false_log_break(d->running, AV_LOG_WARNING, "failed to read a frame.\n");

            if (d->packet->stream_index == d->videoIndex)
            {
                ret = avcodec_send_packet(d->codecCtx, d->packet);
                d->running = (ret >= 0);
                check_false_log_break(d->running, AV_LOG_WARNING, "failed to send packet.\n");

                ret = avcodec_receive_frame(d->codecCtx, d->videoFrame1);
                if (ret == AVERROR(EAGAIN))
                {
                    continue;
                }

                if (ret == AVERROR_EOF)
                {
                    d->running = false;
                    // to be done:
                    // should call avcodec_send_packet() with an empty packet 
                    // to flush out last few frames in the buffer if any.
                    break;
                }

                d->running = (ret >= 0);
                check_false_log_break(d->running, AV_LOG_WARNING, "failed to recieve a new frame.\n");

                d->frame1Ready = true;
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    } while (d->running);
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
        std::lock_guard g(videoMutex);
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
