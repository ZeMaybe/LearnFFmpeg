
#include "SDLWindow.h"
#include "SDLApp.h"
#include "RenderHelper/RenderHelper.h"

SDLWindow::SDLWindow(SDL_Window* wnd)
{
    if (wnd == nullptr)
    {
        SDL_Log("Failed to create SDLWindow from null SDL_Window");
        SDL_assert(false);
    }
    this->wnd = wnd;
    this->wndId = SDL_GetWindowID(wnd);
    bool re = sdlApp->insertWindow(this);
    SDL_assert(re);
}

SDLWindow::SDLWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
{
    if (flags & SDL_WINDOW_OPENGL)
    {
        int ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    }

    wnd = SDL_CreateWindow(title, x, y, w, h, flags);
    if (wnd == nullptr)
    {
        SDL_Log("Failed to create window:%s", SDL_GetError());
        SDL_assert(false);
    }
    wndId = SDL_GetWindowID(wnd);
    bool re = sdlApp->insertWindow(this);
    SDL_assert(re);
}

SDLWindow::~SDLWindow()
{
    if (renderHelper != nullptr)
    {
        delete renderHelper;
    }
    if (wnd != nullptr)
    {
        sdlApp->removeWindow(this);
        SDL_DestroyWindow(wnd);
    }

    wnd = nullptr;
    renderHelper = nullptr;
    wndId = 0;
}

void SDLWindow::onRender()
{
    //if (sdlApp->getRunningTime() - t > 1000.0 / decoder.getFrameRate())
    if (renderHelper != nullptr)
    {
        renderHelper->onRender();
    }
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
