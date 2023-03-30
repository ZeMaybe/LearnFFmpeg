
#pragma once


extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}
#include <mutex>

class FFmpegHwDecoderHelper;
class FFmpegDecoder
{
public:
    FFmpegDecoder(const char* filePath = nullptr,int id = 0,bool hwAccel = true);
    virtual ~FFmpegDecoder();

    bool loadFile(const char* filePath,bool hwAccel = true);
    bool receiveVideoFrame(AVFrame* pFrame);

    AVPixelFormat getOutputFramePixelFormat()const { return outputFramePixelFormat; }
    void setOutputFramePixelFormat(AVPixelFormat newFmt);

    int getPixelWidth()const;
    int getPixelHeight()const;
    void getPixelSize(int* w, int* h)const;
    double getFrameRate()const;

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
    std::mutex video2Mutex;

    std::atomic<bool> running = false;
    std::thread decoderThread;

    friend class FFmpegHwDecoderHelper;

    int Id = 0;
    bool hwAccel = true;

private :
    static void decoderFunction(FFmpegDecoder* decoder);
    int sendPacketAndReceiveFrame(AVPacket* packet);

    FFmpegHwDecoderHelper* helper = nullptr;
};
