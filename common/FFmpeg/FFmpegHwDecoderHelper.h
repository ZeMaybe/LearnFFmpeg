
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

    int getAvaliableHwCount();
    const char* getAvaliableHwName(int index);

    bool setupHwAcceleration();
    bool setupHwAcceleration(const char* hwName);

    static AVPixelFormat getHwFmtCallback(AVCodecContext* codecCtx, const AVPixelFormat* fmts);
protected:
    FFmpegDecoder* master = nullptr;
    AVHWDeviceType hwType = AV_HWDEVICE_TYPE_NONE;
    AVPixelFormat hwPixFmt = AV_PIX_FMT_NONE;
};