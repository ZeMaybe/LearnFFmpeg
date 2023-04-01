
#include "FFmpegDecoder.h"
#include "FFmpegHwDecoderHelper.h"

FFmpegDecoder::FFmpegDecoder()
{
    packet = av_packet_alloc();
}

FFmpegDecoder::FFmpegDecoder(const char* filePath, bool pumpAudio, bool pumpVideo, bool hwAccel, const char* hwName)
    :pumpAudio(pumpAudio)
    , pumpVideo(pumpVideo)
    , hwAccel(hwAccel)
    , hwName(hwName)
{
    packet = av_packet_alloc();
    loadFile(filePath, pumpAudio, pumpVideo, hwAccel, hwName);
}

FFmpegDecoder::~FFmpegDecoder()
{
    clear();
    if (packet != nullptr)
    {
        av_packet_free(&packet);
    }
}

#define check_false_log_return(v,l,t,f)  if(v == false){\
    av_log(nullptr,l,t);\
    f;\
    return v;\
}\

void FFmpegDecoder::setPumpAudio(bool pumpAudio)
{
    if (decoderThread.joinable())
    {
        return;
    }

    this->pumpAudio = pumpAudio;
}

void FFmpegDecoder::setPumpVideo(bool pumpVideo)
{
    if (decoderThread.joinable())
    {
        return;
    }

    this->pumpVideo = pumpVideo;
}

void FFmpegDecoder::setHwAccel(bool hwAccel, const char* hwName)
{
    if (decoderThread.joinable())
    {
        return;
    }

    if (pumpVideo)
    {
        this->hwAccel = hwAccel;
        this->hwName = hwName;
    }
}

bool FFmpegDecoder::loadFile(const char* filePath, bool pumpAudio, bool pumpVideo, bool hwAccel, const char* hwName)
{
    if (filePath == nullptr)        return false;

    clear();

    this->pumpAudio = pumpAudio;
    this->pumpVideo = pumpVideo;
    this->pumpVideo ? this->hwAccel = hwAccel : this->hwAccel = false;
    this->hwName = hwName;

    if (this->pumpAudio == false && this->pumpVideo == false)
    {
        av_log(nullptr, AV_LOG_FATAL, "Are you sure you want pump nothing? Idiot...\n");
        return false;
    }

    bool re = (formatCtx = avformat_alloc_context()) != nullptr;
    check_false_log_return(re, AV_LOG_WARNING, "couldn't allocate AVFormatContext.\n", void);

    re = avformat_open_input(&formatCtx, filePath, nullptr, nullptr) == 0;
    check_false_log_return(re, AV_LOG_WARNING, "couldn't open input file.\n", resetInputFormat());

    re = avformat_find_stream_info(formatCtx, nullptr) >= 0;
    check_false_log_return(re, AV_LOG_WARNING, "couldn't find stream informations.\n", resetInputFormat());

    av_log(nullptr, AV_LOG_INFO, "nb_streams = %d.\n", formatCtx->nb_streams);
    av_dump_format(formatCtx, 0, filePath, false);
    av_log(nullptr, AV_LOG_INFO, "\n");

    re = loadVideoCodec();
    re = loadAudioCodec();

    if (re == true && (pumpVideo == true || pumpAudio == true))
    {
        running = true;
        pauseDecoding = false;
        decoderThread = std::thread(&FFmpegDecoder::run, this);
    }
    else
    {
        av_log(nullptr, AV_LOG_FATAL, "something is wrong when try to load %s\n",filePath);
    }
    return re;
}

void FFmpegDecoder::pause()
{
    pauseDecoding = true;
}

void FFmpegDecoder::resume()
{
    pauseDecoding = false;
}

AVFrame* FFmpegDecoder::getFrame(std::mutex& mutex, std::atomic<bool>& fullFlag, int& maxSize, std::deque<AVFrame*>& queue)
{
    AVFrame* re = nullptr;
    {
        std::lock_guard g(mutex);
        if (queue.size() != 0)
        {
            re = queue.front();
            queue.pop_front();
        }
        if (queue.size() >= maxSize)
        {
            fullFlag = true;
        }
        else
        {
            fullFlag = false;
        }
    }
    return re;
}

AVFrame* FFmpegDecoder::getVideoFrame()
{
    AVFrame* re = nullptr;

    if (pauseDecoding == false)
    {
        re = getFrame(videoMutex, videoFull, videoQueueSize, videoQueue);
    }

    return re;
}

AVFrame* FFmpegDecoder::getAudioFrame()
{
    AVFrame* re = nullptr;

    if (pauseDecoding == false)
    {
        re = getFrame(audioMutex, audioFull, audioQueueSize, audioQueue);
    }

    return re;
}

AVFrame* FFmpegDecoder::getNextVideoFrame()
{
    AVFrame* re = nullptr;
    if (pauseDecoding == true)
    {
        re = getFrame(videoMutex, videoFull, videoQueueSize, videoQueue);
    }
    return re;
}

AVFrame* FFmpegDecoder::getNextAudioFrame()
{
    AVFrame* re = nullptr;
    if (pauseDecoding == true)
    {
        re = getFrame(audioMutex, audioFull, audioQueueSize, audioQueue);
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
    if (videoCodecCtx != nullptr)
    {
        videoCodecCtx->width;
    }
    return re;
}

int FFmpegDecoder::getPixelHeight() const
{
    int re = 0;
    if (videoCodecCtx != nullptr)
    {
        re = videoCodecCtx->height;
    }
    return re;
}

void FFmpegDecoder::getPixelSize(int* w, int* h) const
{
    if (w == nullptr || h == nullptr)
    {
        return;
    }

    if (videoCodecCtx != nullptr)
    {
        *w = videoCodecCtx->width;
        *h = videoCodecCtx->height;
    }
}

double FFmpegDecoder::getFrameRate() const
{
    return av_q2d(videoCodecCtx->framerate);
}

void FFmpegDecoder::resetInputFormat()
{
    stopDecoderThread();
    if (formatCtx != nullptr)
    {
        avformat_close_input(&formatCtx);
        avformat_free_context(formatCtx);
    }
    formatCtx = nullptr;
}

bool FFmpegDecoder::loadCodec(const AVCodec*& codec, AVCodecContext*& ctx, int& index, AVMediaType t)
{
    bool re;
    index = av_find_best_stream(formatCtx, t, -1, -1, &codec, 0);
    re = (index >= 0) && (codec != nullptr);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't find stream.\n", void());

    ctx = avcodec_alloc_context3(codec);
    re = (ctx != nullptr);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't allocate codec context.\n", void());

    re = (avcodec_parameters_to_context(ctx, formatCtx->streams[index]->codecpar) >= 0);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't copy parameters to codec context.\n", void());

    if (t == AVMEDIA_TYPE_VIDEO && hwAccel == true)
    {
        helper = new FFmpegHwDecoderHelper(this);
        re = helper->setupHwAcceleration(hwName);
        check_false_log_return(re, AV_LOG_WARNING, "couldn't setup hardware acceleration.\n", void());
    }

    re = (avcodec_open2(ctx, codec, nullptr) == 0);
    check_false_log_return(re, AV_LOG_WARNING, "couldn't open codec context.\n", void());
    return re;
}

void FFmpegDecoder::resetVideoCodec()
{
    stopDecoderThread();
    if (videoCodecCtx != nullptr)
    {
        avcodec_free_context(&videoCodecCtx);
    }

    videoCodecCtx = nullptr;
    videoCodec = nullptr;
    videoIndex = -1;
    outputFramePixelFormat = AV_PIX_FMT_NONE;

    if (helper != nullptr)
    {
        delete helper;
        helper = nullptr;
    }
}

bool FFmpegDecoder::loadVideoCodec()
{
    if (pumpVideo == false)
    {
        videoFull = true;
        return true;
    }

    bool re = loadCodec(videoCodec, videoCodecCtx, videoIndex, AVMEDIA_TYPE_VIDEO);
    if (re == false)
    {
        pumpVideo = false;
        videoFull = true;
        resetVideoCodec();
    }
    return re;
}

void FFmpegDecoder::resetVideoData()
{
    stopDecoderThread();
    {
        std::lock_guard g(videoMutex);
        while (videoQueue.empty() == false)
        {
            av_frame_free(&(videoQueue.front()));
            videoQueue.pop_front();
        }
        videoFull = false;
    }
}

void FFmpegDecoder::resetAudioCodec()
{
    stopDecoderThread();
    if (audioCodecCtx != nullptr)
    {
        avcodec_free_context(&audioCodecCtx);
    }

    audioCodecCtx = nullptr;
    audioCodec = nullptr;
    audioIndex = -1;
}

bool FFmpegDecoder::loadAudioCodec()
{
    if (pumpAudio == false)
    {
        audioFull = true;
        return true;
    }

    bool re = loadCodec(audioCodec, audioCodecCtx, audioIndex, AVMEDIA_TYPE_AUDIO);
    if (re == false)
    {
        pumpAudio = false;
        audioFull = true;
        resetAudioCodec();
    }
    return re;
}

void FFmpegDecoder::resetAudioData()
{
    stopDecoderThread();

    {
        std::lock_guard g(audioMutex);
        while (audioQueue.empty() == false)
        {
            av_frame_free(&(audioQueue.front()));
            audioQueue.pop_front();
        }
        audioFull = false;
    }
}

void FFmpegDecoder::stopDecoderThread()
{
    if (decoderThread.joinable())
    {
        running = false;
        decoderThread.join();
    }
}

void FFmpegDecoder::run()
{
    av_log(nullptr, AV_LOG_INFO, "decoder thread :%d start running.\n", wndId);
    bool re = true;
    while (running)
    {
        if (re == false)
        {
            sendVideoPacket(nullptr);
            sendAudioPacket(nullptr);
            running = false;
            continue;
        }

        if (videoFull == false || audioFull == false)
        {
            av_packet_unref(packet);
            if (av_read_frame(formatCtx, packet) < 0)
            {
                sendVideoPacket(nullptr);
                sendAudioPacket(nullptr);
                running = false;
                continue;
            }

            if (packet->stream_index == videoIndex)
            {
                re = sendVideoPacket(packet);
            }
            else if (packet->stream_index == audioIndex)
            {
                re = sendAudioPacket(packet);
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    av_log(nullptr, AV_LOG_INFO, "decoder thread :%d stop running.\n", wndId);
}

bool FFmpegDecoder::sendPacket(AVPacket* p, AVCodecContext*& ctx, std::deque<AVFrame*>& queue, std::atomic<bool>& full, std::mutex& mutex, const int& maxSize)
{
    int ret = avcodec_send_packet(ctx, p);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_WARNING, "%d:Can't send packet.\n", wndId);
        return false;
    }

    while (running)
    {
        AVFrame* tmp = av_frame_alloc();
        ret = avcodec_receive_frame(ctx, tmp);
        if (ret == AVERROR(EAGAIN))
        {
            return true;
        }

        if (ret == AVERROR_EOF)
        {
            return false;
        }

        if (ret < 0)
        {
            av_log(nullptr, AV_LOG_WARNING, "%d:Can't receive frame.\n", wndId);
            return false;
        }

        {
            std::lock_guard g(mutex);
            queue.push_back(tmp);
            if (queue.size() >= maxSize)
            {
                full = true;
            }
        }
    }

    return false;
}

bool FFmpegDecoder::sendVideoPacket(AVPacket* p)
{
    if (pumpVideo == false)
    {
        return true;
    }
    return sendPacket(p, videoCodecCtx, videoQueue, videoFull, videoMutex, videoQueueSize);
}

bool FFmpegDecoder::sendAudioPacket(AVPacket* p)
{
    if (pumpAudio == false)
    {
        return true;
    }
    return sendPacket(p, audioCodecCtx, audioQueue, audioFull, audioMutex, audioQueueSize);
}

void FFmpegDecoder::clear()
{
    resetVideoData();
    resetVideoCodec();

    resetAudioData();
    resetAudioCodec();
}
