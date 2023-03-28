
#include "SDLRenderHelper.h"
#include "convert.h"
extern "C"
{
#include "libavcodec/avcodec.h"
}

SDLRenderHelper::SDLRenderHelper(SDL_Window* w)
    :RenderHelper(w)
{
    createRenderer();
}

SDLRenderHelper::~SDLRenderHelper()
{
    clear();
}

void SDLRenderHelper::update(AVFrame* frame)
{
    if (frame == nullptr)   return;
    if (texture == nullptr)
    {
        textureFmt = pixelFmtFFmpegToSDL((AVPixelFormat)frame->format);
        createTexture(textureFmt,frame->width, frame->height);
    }

    if (textureFmt == SDL_PIXELFORMAT_IYUV)
    {
        SDL_UpdateYUVTexture(texture,nullptr,frame->data[0],frame->linesize[0],
            frame->data[1],frame->linesize[1],
            frame->data[2],frame->linesize[1]);
        return;
    }

    if (textureFmt == SDL_PIXELFORMAT_NV12 ||
        textureFmt == SDL_PIXELFORMAT_NV21)
    {
        SDL_UpdateNVTexture(texture,nullptr,frame->data[0],frame->linesize[0],frame->data[1],frame->linesize[1]);
        return;
    }
}

void SDLRenderHelper::onRender()
{
    if (SDL_GetWindowFlags(wnd) & SDL_WINDOW_HIDDEN)
    {
        return;
    }

    SDL_Rect rect = { 0 };
    SDL_GetWindowSize(wnd, &rect.w, &rect.h);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_RenderPresent(renderer);
}

void SDLRenderHelper::createRenderer()
{
    if (wnd == nullptr)        return;
    if ((SDL_GetWindowFlags(wnd) & SDL_WINDOW_OPENGL) == 0)        return;
    clear();

    renderer = SDL_CreateRenderer(wnd, getRenderDriverIndex(), SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    SDL_Log("Current render info:%s\n", info.name);
    SDL_HINT_RENDER_BATCHING;
}

void SDLRenderHelper::createTexture(SDL_PixelFormatEnum fmt, int w, int h, int access)
{
    SDL_DestroyTexture(texture);
    texture = SDL_CreateTexture(renderer, fmt, access, w, h);
}

int SDLRenderHelper::getRenderDriverIndex()const
{
    SDL_RendererInfo info;
    int count = SDL_GetNumRenderDrivers();
    for (int i = 0; i < count; ++i)
    {
        SDL_GetRenderDriverInfo(i, &info);
        if (SDL_strcmp(info.name, "opengl") == 0)
        {
            return i;
        }
    }
    return -1;
}

void SDLRenderHelper::clear()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);

    renderer = nullptr;
    texture = nullptr;
}
