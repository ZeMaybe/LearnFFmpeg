
#include "FFmpegHwDecoderHelper.h"
#include "FFmpegDecoder.h"

FFmpegHwDecoderHelper::FFmpegHwDecoderHelper(FFmpegDecoder* decoder)
    :master(decoder)
{
}

FFmpegHwDecoderHelper::~FFmpegHwDecoderHelper()
{
    master = nullptr;
}

int FFmpegHwDecoderHelper::getAvaliableHwCount()
{
    AVHWDeviceType t = AV_HWDEVICE_TYPE_NONE;
    int count = 0;

    while ((t = av_hwdevice_iterate_types(t)) != AV_HWDEVICE_TYPE_NONE)
    {
        ++count;
    }

    return count;
}

const char* FFmpegHwDecoderHelper::getAvaliableHwName(int index)
{
    const char* re = nullptr;
    if (index >= 0)
    {
        AVHWDeviceType t = AV_HWDEVICE_TYPE_NONE;
        int i = 0;
        while ((t = av_hwdevice_iterate_types(t)) != AV_HWDEVICE_TYPE_NONE)
        {
            if (index == i)
            {
                re = av_hwdevice_get_type_name(t);
                break;
            }

            ++i;
        }
    }
    return re;
}

bool FFmpegHwDecoderHelper::isHwAccelerationAvailable(const char* hwName)
{
    bool re = false;
    if (getAvaliableHwCount() != 0 && hwName != 0)
    {
        auto hwType = av_hwdevice_find_type_by_name(hwName);
        re = (hwType != AV_HWDEVICE_TYPE_NONE);
    }
    return re;
}

bool FFmpegHwDecoderHelper::setupHwAcceleration()
{
    bool re = false;
    if (getAvaliableHwCount() != 0)
    {
        re = setupHwAcceleration(getAvaliableHwName(0));
    }
    return re;
}

bool FFmpegHwDecoderHelper::setupHwAcceleration(const char* hwDeviceName)
{
    hwType = av_hwdevice_find_type_by_name(hwDeviceName);
    if (hwType == AV_HWDEVICE_TYPE_NONE)        return false;

    for (int i = 0, index = 0; ; ++i)
    {
        auto config = avcodec_get_hw_config(master->videoCodec, i);
        if (config == nullptr) break;

        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == hwType)
        {
            hwPixFmt = config->pix_fmt;
            break;
        }
    }

    master->videoCodecCtx->opaque = this;
    master->videoCodecCtx->get_format = getHwFmtCallback;
    int ret = av_hwdevice_ctx_create(&(master->videoCodecCtx->hw_device_ctx), hwType, nullptr, nullptr, 0);
    if (ret != 0)
    {
        char tmp[1024] = { 0 };
        av_strerror(ret, tmp, 1024);
        av_log(nullptr, AV_LOG_INFO, "can't create hwdevice:%s", tmp);
        return false;
    }
    return true;
}

AVPixelFormat FFmpegHwDecoderHelper::getHwFmtCallback(AVCodecContext* codecCtx, const AVPixelFormat* fmts)
{
    FFmpegHwDecoderHelper* helper = (FFmpegHwDecoderHelper*)codecCtx->opaque;
    return helper->hwPixFmt;
}
