
#include "SDLApp.h"
#include <iostream>
#include "SDLWindow.h"

SDLApp* SDLApp::theApp = nullptr;

SDLApp* SDLApp::get()
{
    SDL_assert(theApp != nullptr);
    return theApp;
}

SDLApp::SDLApp(Uint32 sdlFlags)
{
    if (theApp != nullptr)
    {
        SDL_Log("Only one SDLApp instance is allowed.");
        SDL_assert(false);
    }
    theApp = this;

    running = init(sdlFlags);
    if (running == false)
    {
        std::cout << "SDL2 init failed! exec() will does nothing!";
    }
}

int SDLApp::exec()
{
    SDL_Event Event;
    while (running)
    {
        while (SDL_PollEvent(&Event))
        {
            dispatchEvent(&Event);
        }

        loop();
        render();
    }
    clean();

    return 0;
}

bool SDLApp::init(Uint32 flags)
{
    if (SDL_Init(flags) < 0)
    {
        std::cout << "Failed to init sdl:" << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

void SDLApp::clean()
{
    running = false;
    SDL_Quit();
}

void SDLApp::dispatchEvent(SDL_Event* ev)
{
    switch (ev->type)
    {
    case SDL_DISPLAYEVENT:        dispatchDisplayEvent(ev->display);    break;
    case SDL_WINDOWEVENT:         dispatchWindowEvent(ev->window);      break;
    case SDL_KEYDOWN:             dispatchKeyDownEvent(ev->key);        break;
    case SDL_KEYUP:               dispatchKeyUpEvent(ev->key);          break;
    case SDL_TEXTEDITING:         dispatchTextEditingEvent(ev->edit);   break;
    case SDL_TEXTINPUT:           dispatchTextInputEvent(ev->text);     break;
    case SDL_MOUSEMOTION:         dispatchMouseMotionEvent(ev->motion);    break;
    case SDL_MOUSEBUTTONDOWN:     dispatchMouseButtonDownEvent(ev->button); break;
    case SDL_MOUSEBUTTONUP:       dispatchMouseButtonUpEvent(ev->button);   break;
    case SDL_MOUSEWHEEL:          dispatchMouseWheelEvent(ev->wheel);       break;
    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
    case SDL_JOYHATMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    case SDL_JOYDEVICEADDED:
    case SDL_JOYDEVICEREMOVED:
    case SDL_CONTROLLERAXISMOTION:
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERDEVICEREMAPPED:
        SDL_Log("Joy and Controller event not handled!");
        break;
    case SDL_AUDIODEVICEADDED:        onAudioDeviceAdded(ev->adevice.which, ev->adevice.iscapture);   break;
    case SDL_AUDIODEVICEREMOVED:      onAudioDeviceRemoved(ev->adevice.which, ev->adevice.iscapture); break;
    case SDL_QUIT:                    onQuit(); break;
    case SDL_USEREVENT:               dispatchUserEvent(ev->user); break;
    case SDL_SYSWMEVENT:              SDL_Log("SystemWmEvent not handled!"); break;
    case SDL_FINGERMOTION:
    case SDL_FINGERDOWN:
    case SDL_FINGERUP:
    case SDL_MULTIGESTURE:
    case SDL_DOLLARGESTURE:
    case SDL_DOLLARRECORD:            SDL_Log("Finger and Gesture event not handled!");  break;
    case SDL_DROPBEGIN:
    case SDL_DROPCOMPLETE:
    case SDL_DROPFILE:
    case SDL_DROPTEXT:                SDL_Log("Drop event not handled!");       break;
    default:                          SDL_Log("Unknown event : %d!", ev->type);  break;
    }
}

void SDLApp::dispatchDisplayEvent(const SDL_DisplayEvent& ev)
{
    switch (ev.event)
    {
    case SDL_DISPLAYEVENT_CONNECTED:     onDisplayConnected(ev.display); break;
    case SDL_DISPLAYEVENT_DISCONNECTED:  onDisplayDisconnected(ev.display); break;
    case SDL_DISPLAYEVENT_ORIENTATION:   onDisplayOrientationChanged(ev.display, ev.data1); break;
    }
}

void SDLApp::dispatchWindowEvent(const SDL_WindowEvent& ev)
{
    SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
    if (wnd == nullptr)        return;

    switch (ev.event)
    {
    case SDL_WINDOWEVENT_SHOWN:        onWindowShown(wnd);  break;
    case SDL_WINDOWEVENT_HIDDEN:       onWindowHidden(wnd); break;
    case SDL_WINDOWEVENT_EXPOSED:      onWindowExposed(wnd);    break;
    case SDL_WINDOWEVENT_MOVED:        onWindowMoved(wnd, ev.data1, ev.data2);       break;
    case SDL_WINDOWEVENT_RESIZED:      onWindowResized(wnd, ev.data1, ev.data2);     break;
    case SDL_WINDOWEVENT_SIZE_CHANGED: onWindowSizeChanged(wnd, ev.data1, ev.data2); break;
    case SDL_WINDOWEVENT_MINIMIZED:    onWindowMinimized(wnd);       break;
    case SDL_WINDOWEVENT_MAXIMIZED:    onWindowMaximized(wnd);       break;
    case SDL_WINDOWEVENT_RESTORED:     onWindowRestored(wnd);    break;
    case SDL_WINDOWEVENT_ENTER:        onWindowMouseEnter(wnd);  break;
    case SDL_WINDOWEVENT_LEAVE:        onWindowMouseLeave(wnd);  break;
    case SDL_WINDOWEVENT_FOCUS_GAINED: onWindowKeyboardFocusGained(wnd); break;
    case SDL_WINDOWEVENT_FOCUS_LOST:   onWindowKeyboardFocusLost(wnd);   break;
    case SDL_WINDOWEVENT_CLOSE:        onWindowClose(wnd); break;
#if SDL_VERSION_ATLEAST(2,0,5)
    case SDL_WINDOWEVENT_TAKE_FOCUS:   onWindowTakeFocus(wnd); break;
    case SDL_WINDOWEVENT_HIT_TEST:     onWindowHitTest(wnd);   break;
#endif

#if SDL_VERSION_ATLEAST(2,0,18)
    case SDL_WINDOWEVENT_ICCPROF_CHANGED: onWindowIccprofChanged(wnd); break;
    case SDL_WINDOWEVENT_DISPLAY_CHANGED: onWindowDisplayChanged(wnd, ev.data1); break;
#endif
    default: SDL_Log("Unhandled SDL_WINDOWEVENT for window : %d", ev.windowID); break;
    }
}

void SDLApp::dispatchKeyDownEvent(const SDL_KeyboardEvent& ev)
{
    if (onKeyDown(ev.keysym, ev.repeat) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowKeyDown(wnd, ev.keysym, ev.repeat);
        }
    }
}

void SDLApp::dispatchKeyUpEvent(const SDL_KeyboardEvent& ev)
{
    if (onKeyUp(ev.keysym, ev.repeat) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowKeyUp(wnd, ev.keysym, ev.repeat);
        }
    }
}

void SDLApp::dispatchTextEditingEvent(const SDL_TextEditingEvent& ev)
{
    if (onTextEditing(ev.text, ev.start, ev.length) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowTextEditing(wnd, ev.text, ev.start, ev.length);
        }
    }
}

void SDLApp::dispatchTextInputEvent(const SDL_TextInputEvent& ev)
{
    if (onTextInput(ev.text) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowTextInput(wnd, ev.text);
        }
    }
}

//https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent
void SDLApp::dispatchMouseMotionEvent(const SDL_MouseMotionEvent& ev)
{
    if (onMouseMove(ev.which, ev.state, ev.x, ev.y, ev.xrel, ev.yrel) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowMouseMove(wnd, ev.which, ev.state, ev.x, ev.y, ev.xrel, ev.yrel);
        }
    }
}

void SDLApp::dispatchMouseButtonDownEvent(const SDL_MouseButtonEvent& ev)
{
    if (onMouseButtonDown(ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowMouseButtonDown(wnd, ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y);
        }
    }
}

void SDLApp::dispatchMouseButtonUpEvent(const SDL_MouseButtonEvent& ev)
{
    if (onMouseButtonUp(ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowMouseButtonUp(wnd, ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y);
        }
    }
}

void SDLApp::dispatchMouseWheelEvent(const SDL_MouseWheelEvent& ev)
{
    if (onMouseWheel(ev.which, ev.x, ev.y, ev.direction, ev.preciseX, ev.preciseY) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowMouseWheel(wnd, ev.which, ev.x, ev.y, ev.direction, ev.preciseX, ev.preciseY);
        }
    }
}

void SDLApp::dispatchUserEvent(const SDL_UserEvent& ev)
{
    if (onUserEvent(ev.code, ev.data1, ev.data2) == false)
    {
        SDL_Window* wnd = SDL_GetWindowFromID(ev.windowID);
        if (wnd != nullptr)
        {
            onWindowUserEvent(wnd, ev.code, ev.data1, ev.data2);
        }
    }
}

bool SDLApp::insertWindow(SDLWindow* wnd)
{
    bool re = false;
    if (wnd != nullptr)
    {
        if(wnds.count(wnd->getWndId()) == 0)
        {
           re = (wnds.insert(std::pair{ wnd->getWndId(), wnd })).second;
        }
    }
    return re;
}

