
#pragma once

struct SDL_Window;
struct AVFrame;
class RenderHelper
{
public:
    RenderHelper(SDL_Window* w) :wnd(w) {};
    virtual ~RenderHelper() {}

    virtual void update(AVFrame* frame) = 0;
    virtual void onRender() = 0;

protected:
    SDL_Window* wnd = nullptr;
};
