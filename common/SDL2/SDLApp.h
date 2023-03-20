
#pragma once

extern "C"
{
#include "SDL.h"
}

class SDLApp 
{
public:
    SDLApp(Uint32 sdlFlags);
    int exec();

protected:
    virtual bool init(Uint32 initFlags);
    virtual void clean();
    //virtual void loop() = 0;
    //virtual void render() = 0;
    virtual void loop() {};
    virtual void render() {};

protected:
    void dispatchEvent(SDL_Event* ev);
    void dispatchDisplayEvent(const SDL_DisplayEvent& ev);
    void dispatchWindowEvent(const SDL_WindowEvent& ev);
    void dispatchKeyDownEvent(const SDL_KeyboardEvent& ev);
    void dispatchKeyUpEvent(const SDL_KeyboardEvent& ev);
    void dispatchTextEditingEvent(const SDL_TextEditingEvent& ev);
    void dispatchTextInputEvent(const SDL_TextInputEvent& ev);
    void dispatchMouseMotionEvent(const SDL_MouseMotionEvent& ev);
    void dispatchMouseButtonDownEvent(const SDL_MouseButtonEvent& ev);
    void dispatchMouseButtonUpEvent(const SDL_MouseButtonEvent& ev);
    void dispatchMouseWheelEvent(const SDL_MouseWheelEvent& ev);

    // display event
    virtual void onDisplayConnected(Uint32 displayIndex) {}
    virtual void onDisplayDisconnected(Uint32 displayIndex) {}
    virtual void onDisplayOrientationChanged(Uint32 displayIndex, Sint32 orientation) {}

    // window event
    virtual void onWindowShown(SDL_Window* wnd) {}
    virtual void onWindowHidden(SDL_Window* wnd) {}
    virtual void onWindowExposed(SDL_Window* wnd) {}
    virtual void onWindowMoved(SDL_Window* wnd, Sint32 x, Sint32 y) {}
    virtual void onWindowResized(SDL_Window* wnd, Sint32 w, Sint32 h) {}
    virtual void onWindowSizeChanged(SDL_Window* wnd, Sint32 w, Sint32 h) {}
    virtual void onWindowMinimized(SDL_Window* wnd) {}
    virtual void onWindowMaximized(SDL_Window* wnd) {}
    virtual void onWindowRestored(SDL_Window* wnd) {}
    virtual void onWindowMouseEnter(SDL_Window* wnd) {}
    virtual void onWindowMouseLeave(SDL_Window* wnd) {}
    virtual void onWindowKeyboardFocusGained(SDL_Window* wnd) {}
    virtual void onWindowKeyboardFocusLost(SDL_Window* wnd) {}
    virtual void onWindowClose(SDL_Window* wnd) {}
    virtual void onWindowTakeFocus(SDL_Window* wnd) {}
    virtual void onWindowHitTest(SDL_Window* wnd) {}
    virtual void onWindowIccprofChanged(SDL_Window* wnd) {}
    virtual void onWindowDisplayChanged(SDL_Window* wnd,Sint32 displayIndex) {}

    // return true,this event will stop being handled further.
    virtual bool onKeyDown(SDL_Keysym key,Uint8 repeat) { return false; };
    virtual bool onKeyUp(SDL_Keysym key,Uint8 repeat) { return false; }
    virtual void onWindowKeyDown(SDL_Window* wnd, SDL_Keysym key,Uint8 repeat) {}
    virtual void onWindowKeyUp(SDL_Window* wnd, SDL_Keysym key,Uint8 repeat) {}

    // text eidting
    virtual bool onTextEditing(const char* text,Sint32 start,Sint32 len) { return false; }
    virtual void onWindowTextEditing(SDL_Window* wnd,const char* text, Sint32 start, Sint32 len) {}

    // text input
    virtual bool onTextInput(const char* text) { return false; }
    virtual void onWindowTextInput(SDL_Window* wnd, const char* text) {}

    // mouse motion
    virtual bool onMouseMove(Uint32 mouseId,Uint32 state,Sint32 x,Sint32 y,Sint32 xrel,Sint32 yrel) { return false; }
    virtual void onWindowMouseMove(SDL_Window* wnd, Uint32 mouseId, Uint32 state, Sint32 x, Sint32 y, Sint32 xrel, Sint32 yrel) {}

    // mouse button 
    virtual bool onMouseButtonDown(Uint32 mouseId,Uint8 button,Uint8 state,Uint8 clicks,Sint32 x,Sint32 y) { return false; }
    virtual bool onMouseButtonUp(Uint32 mouseId,Uint8 button,Uint8 state,Uint8 clicks,Sint32 x,Sint32 y) { return false; }
    virtual void onWindowMouseButtonDown(SDL_Window* wnd, Uint32 mouseId,Uint8 button,Uint8 state,Uint8 clicks,Sint32 x,Sint32 y) {}
    virtual void onWindowMouseButtonUp(SDL_Window* wnd,Uint32 mouseId,Uint8 button,Uint8 state,Uint8 clicks,Sint32 x,Sint32 y) {}

    // mouse wheel
    virtual bool onMouseWheel(Uint32 mouseId,Sint32 x,Sint32 y,Uint32 direction,float preciseX,float preciseY) { return false; }
    virtual void onWindowMouseWheel(SDL_Window* wnd, Uint32 mouseId,Sint32 x,Sint32 y,Uint32 direction,float preciseX,float preciseY) {}

    // audio device
    virtual void onAudioDevice(Uint32 device, bool isCapture) {}


protected:
    bool running = true;

    SDL_Window* wnd = nullptr;
};
