
#pragma once

extern "C"
{
#include "libavcodec/avcodec.h"
}

class FFmpegDecoder;
class FFmpegHwDecoderHelper
{
public:
    FFmpegHwDecoderHelper(FFmpegDecoder* master);
    virtual ~FFmpegHwDecoderHelper();

    static int getAvaliableHwCount();
    static const char* getAvaliableHwName(int index);
    static bool isHwAccelerationAvailable(const char* hwName);

    bool setupHwAcceleration();
    bool setupHwAcceleration(const char* hwName);

    static AVPixelFormat getHwFmtCallback(AVCodecContext* codecCtx, const AVPixelFormat* fmts);
protected:
    FFmpegDecoder* master = nullptr;
    AVHWDeviceType hwType = AV_HWDEVICE_TYPE_NONE;
    AVPixelFormat hwPixFmt = AV_PIX_FMT_NONE;
};