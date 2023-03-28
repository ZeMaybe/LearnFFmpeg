
#pragma once

extern "C"
{
#include "SDL.h"
}

class SDLWindow
{
public:
    SDLWindow(SDL_Window* wnd);
    SDLWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
    virtual ~SDLWindow();

    SDL_Window* getWnd() { return wnd; }
    Uint32 getWndId()const { return wndId; }

    bool getDestroyOnClose() const { return destroyOnClose; }
    void setDestroyOnClose(bool destroy) { destroyOnClose = destroy; }

protected:
    friend class SDLApp;
    virtual void onTick() {}
    virtual void onRender();

    virtual void onShown() {}
    virtual void onHidden() {}
    virtual void onExposed() {}
    virtual void onMoved(Sint32 x, Sint32 y) {}
    virtual void onResized(Sint32 w, Sint32 h) {}
    virtual void onSizeChanged(Sint32 w, Sint32 h) {}
    virtual void onMinimized() {}
    virtual void onMaximized() {}
    virtual void onRestored() {}
    virtual void onMouseEnter() {}
    virtual void onMouseLeave() {}
    virtual void onKeyboardFocusGained() {}
    virtual void onKeyboardFocusLost() {}
    virtual void onClose();
    virtual void onTakeFocus() {}
    virtual void onHitTest() {}
    virtual void onIccprofChanged() {}
    virtual void onDisplayChanged(Sint32 displayIndex) {}
    virtual void onKeyDown(SDL_Keysym key, Uint8 repeat) {}
    virtual void onKeyUp(SDL_Keysym key, Uint8 repeat) {}
    virtual void onTextEditing(const char* text, Sint32 start, Sint32 len) {}
    virtual void onTextInput(const char* text) {}
    virtual void onMouseMove(Uint32 mouseId, Uint32 state, Sint32 x, Sint32 y, Sint32 xrel, Sint32 yrel) {}
    virtual void onMouseButtonDown(Uint32 mouseId, Uint8 button, Uint8 state, Uint8 clicks, Sint32 x, Sint32 y) {}
    virtual void onMouseButtonUp(Uint32 mouseId, Uint8 button, Uint8 state, Uint8 clicks, Sint32 x, Sint32 y) {}
    virtual void onMouseWheel(Uint32 mouseId, Sint32 x, Sint32 y, Uint32 direction, float preciseX, float preciseY) {}
    virtual void onUserEvent(Sint32 code, void* data1, void* data2) {}

protected:
    Uint32 wndId = 0;
    SDL_Window* wnd = nullptr;

    class RenderHelper* renderHelper = nullptr;

    bool destroyOnClose = true;
    long long t = 0;
};
