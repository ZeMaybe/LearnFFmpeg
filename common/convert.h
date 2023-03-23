
#pragma once

extern "C"
{
#include "SDL_pixels.h"
#include "libavutil/pixfmt.h"
}

SDL_PixelFormatEnum pixelFmtFFmpegToSDL(AVPixelFormat fmt);
AVPixelFormat pixelFmtSDLToFFmpeg(SDL_PixelFormatEnum fmt);