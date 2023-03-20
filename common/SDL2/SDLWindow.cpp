
#include "SDLWindow.h"

SDLWindow::SDLWindow(const char* title,int x,int y,int w,int h,Uint32 flags)
{
    wnd = SDL_CreateWindow(title, x, y, w, h, flags);
    if (wnd == nullptr)
    {
        SDL_Log("Failed to create window:%s",SDL_GetError());
        SDL_assert(false);
    }
    wndId = SDL_GetWindowID(wnd);
}

SDLWindow::~SDLWindow()
{
    if (wnd != nullptr)
    {
        SDL_DestroyWindow(wnd);
    }
    wnd = nullptr;
    wndId = 0;
}