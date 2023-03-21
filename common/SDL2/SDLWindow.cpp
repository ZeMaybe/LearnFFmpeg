
#include "SDLWindow.h"
#include "SDLApp.h"

SDLWindow::SDLWindow(SDL_Window* wnd)
{
    if (wnd == nullptr)
    {
        SDL_Log("Failed to create SDLWindow from null SDL_Window");
        SDL_assert(false);
    }
    this->wnd = wnd;
    this->wndId = SDL_GetWindowID(wnd);
    SDL_assert(sdlApp->insertWindow(this));
    createRenderer();
    createTexture(SDL_PIXELFORMAT_IYUV);
}

SDLWindow::SDLWindow(const char* title,int x,int y,int w,int h,Uint32 flags)
{
    wnd = SDL_CreateWindow(title, x, y, w, h, flags);
    if (wnd == nullptr)
    {
        SDL_Log("Failed to create window:%s",SDL_GetError());
        SDL_assert(false);
    }
    wndId = SDL_GetWindowID(wnd);
    SDL_assert(sdlApp->insertWindow(this));
    createRenderer();
    createTexture(SDL_PIXELFORMAT_IYUV);
}

SDLWindow::~SDLWindow()
{
    if (wnd != nullptr)
    {
        sdlApp->removeWindow(this);
        SDL_DestroyWindow(wnd);
    }
    setRenderer(nullptr);
    setTexture(nullptr);

    wnd = nullptr;
    wndId = 0;
}

bool SDLWindow::createRenderer(Uint32 flags)
{
    auto r = SDL_CreateRenderer(wnd, -1, flags);
    setRenderer(r);
    return renderer != nullptr;
}

bool SDLWindow::createTexture(Uint32 format, int access)
{
    int w, h;
    SDL_GetWindowSize(wnd,&w,&h);
    return createTexture(format, w, h, access);
}

bool SDLWindow::createTexture(Uint32 format,int w,int h, int access)
{
    auto t = SDL_CreateTexture(renderer, format, access, w,h);
    setTexture(t);
    return texture != nullptr;
}

void SDLWindow::setRenderer(SDL_Renderer* r)
{
    if (renderer != nullptr)
    {
        SDL_DestroyRenderer(renderer);
    }
    renderer = r;
}

void SDLWindow::setTexture(SDL_Texture* t)
{
    if (texture != nullptr)
    {
        SDL_DestroyTexture(texture);
    }
    texture = t;
}

void SDLWindow::onLoop()
{
}

void SDLWindow::onRender()
{
    // window is hidden,does nothing...
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

void SDLWindow::onClose()
{
    if (destroyOnClose)
    {
        delete this;
    }
    else
    {
        SDL_HideWindow(wnd);
    }
}
