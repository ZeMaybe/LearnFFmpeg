
#pragma once

#include <mutex>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

class FFmpegDecoder
{
public:
    FFmpegDecoder(const char* filePath = nullptr);
    virtual ~FFmpegDecoder();

    bool loadFile(const char* filePath);
    bool receiveVideoFrame(AVFrame* pFrame);

    AVPixelFormat getOutputFramePixelFormat()const { return outputFramePixelFormat; }
    void setOutputFramePixelFormat(AVPixelFormat newFmt);

    int getPixelWidth()const;
    int getPixelHeight()const;
    void getPixelSize(int* w, int* h)const;
    double getFrameRate()const;

    // decoder thread.
    static void decoderFunction(FFmpegDecoder* decoder);
protected:
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    const AVCodec* codec = nullptr;
    int videoIndex = -1;
    AVPixelFormat outputFramePixelFormat = AV_PIX_FMT_NONE;
    void resetCodec();

    AVPacket* packet = nullptr;
    AVFrame* videoFrame1 = nullptr;
    AVFrame* videoFrame2 = nullptr;
    void resetInternalData();
    void clearInternalData();

    std::atomic<bool> frame1Ready = false;
    std::atomic<bool> frame2Ready = false;
    std::mutex videoMutex;

    std::atomic<bool> running = false;
    std::thread decoderThread;
};