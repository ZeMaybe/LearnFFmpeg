
#pragma once

extern "C"
{
#include "SDL.h"
}

class SDLWindow
{
public:
    SDLWindow(const char* title,int x,int y,int w,int h,Uint32 flags);
    virtual ~SDLWindow();

    SDL_Window* getWnd() { return wnd; }
    Uint32 getWndId()const { return wndId; }

protected:
    SDL_Window* wnd = nullptr;
    Uint32 wndId = 0;
};
