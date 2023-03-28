
#pragma once
#include "RenderHelper.h"

// use SDL_Renderer and SDLTexture.

extern "C"
{
#include "SDL.h"
}

class SDLRenderHelper : public RenderHelper
{
public:
    SDLRenderHelper(SDL_Window* w);
    virtual ~SDLRenderHelper();

    virtual void update(struct AVFrame* frame)override;
    virtual void onRender()override;

protected:
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_PixelFormatEnum textureFmt = SDL_PIXELFORMAT_UNKNOWN;

    void createRenderer();
    void createTexture(SDL_PixelFormatEnum fmt,int w,int h,int access = SDL_TEXTUREACCESS_STREAMING);

    int getRenderDriverIndex()const;

    void clear();
};
