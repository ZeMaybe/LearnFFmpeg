
#include "convert.h"

SDL_PixelFormatEnum pixelFmtFFmpegToSDL(AVPixelFormat fmt)
{
	switch (fmt)
	{
	case AV_PIX_FMT_YUV420P: return SDL_PIXELFORMAT_IYUV;
	case AV_PIX_FMT_NV12:    return SDL_PIXELFORMAT_NV12;
	case AV_PIX_FMT_NV21:    return SDL_PIXELFORMAT_NV21;
	}

	return SDL_PIXELFORMAT_UNKNOWN;
}

AVPixelFormat pixelFmtSDLToFFmpeg(SDL_PixelFormatEnum fmt)
{
	switch (fmt)
	{
	case SDL_PIXELFORMAT_IYUV: return AV_PIX_FMT_YUV420P;    // yuv420 : y-plane,u-plane,v-plane
	case SDL_PIXELFORMAT_NV12: return AV_PIX_FMT_NV12;       // yuv420 : y-plane,u/v-plane
	case SDL_PIXELFORMAT_NV21: return AV_PIX_FMT_NV21;       // yuv420 : y-plane,v/u-plane
	}
    return AV_PIX_FMT_NONE;
}
