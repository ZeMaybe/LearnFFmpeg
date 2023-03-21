
#pragma once

extern "C"
{
#include "SDL.h"
}

#include <map>
#include <chrono>

#define sdlApp (SDLApp::get())

class SDLWindow;
class SDLApp
{
public:
    SDLApp(Uint32 sdlFlags);
    int exec();
    static SDLApp* get();
    long long getRunningTime()const;

protected:
    // overide these functions in your subclass
    virtual bool init(Uint32 initFlags);
    virtual void tick();
    virtual void render();
    virtual void clean();

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
    void dispatchUserEvent(const SDL_UserEvent& ev);

    // override these functions in your subclass as you need.
    // display event
    virtual void onDisplayConnected(Uint32 displayIndex) {}
    virtual void onDisplayDisconnected(Uint32 displayIndex) {}
    virtual void onDisplayOrientationChanged(Uint32 displayIndex, Sint32 orientation) {}

    // return true,this event will stop being handled further.
    virtual bool onKeyDown(SDL_Keysym key, Uint8 repeat) { return false; };
    virtual bool onKeyUp(SDL_Keysym key, Uint8 repeat) { return false; }
    // text eidting
    virtual bool onTextEditing(const char* text, Sint32 start, Sint32 len) { return false; }
    // text input
    virtual bool onTextInput(const char* text) { return false; }
    // mouse motion
    virtual bool onMouseMove(Uint32 mouseId, Uint32 state, Sint32 x, Sint32 y, Sint32 xrel, Sint32 yrel) { return false; }

    // mouse button 
    virtual bool onMouseButtonDown(Uint32 mouseId, Uint8 button, Uint8 state, Uint8 clicks, Sint32 x, Sint32 y) { return false; }
    virtual bool onMouseButtonUp(Uint32 mouseId, Uint8 button, Uint8 state, Uint8 clicks, Sint32 x, Sint32 y) { return false; }
    // mouse wheel
    virtual bool onMouseWheel(Uint32 mouseId, Sint32 x, Sint32 y, Uint32 direction, float preciseX, float preciseY) { return false; }
    // audio device
    virtual void onAudioDeviceAdded(Uint32 device, bool isCapture) {}
    virtual void onAudioDeviceRemoved(Uint32 device, bool isCapture) {}
    // quite
    virtual void onQuit();
    // user custom event
    virtual bool onUserEvent(Sint32 code, void* data1, void* data2) { return false; }
    
    SDLWindow* getWnd(Uint32 wndId);

protected:
    bool running = true;

    static SDLApp* theApp;

    friend class SDLWindow;
    std::map<Uint32, SDLWindow*> wnds;

    SDLWindow* lastActiveWnd = nullptr;
    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
protected:
    bool insertWindow(SDLWindow* wnd);
    void removeWindow(SDLWindow* wnd);
};
