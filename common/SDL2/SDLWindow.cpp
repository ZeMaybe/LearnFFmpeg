
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
}

SDLWindow::~SDLWindow()
{
    if (wnd != nullptr)
    {
        sdlApp->removeWindow(this);
        SDL_DestroyWindow(wnd);
    }
    wnd = nullptr;
    wndId = 0;
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
