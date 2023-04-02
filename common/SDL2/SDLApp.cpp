
#include "SDLApp.h"
#include <iostream>
#include "SDLWindow.h"

SDLApp* SDLApp::theApp = nullptr;

SDLApp* SDLApp::get()
{
    SDL_assert(theApp != nullptr);
    return theApp;
}

long long SDLApp::getRunningTime() const
{
    return (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t)).count();
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
    wnds.reserve(32);
}

int SDLApp::exec()
{
    t = std::chrono::system_clock::now();
    if (wnds.size() == 0)
    {
        running = false;
    }

    SDL_Event Event;
    while (running)
    {
        while (SDL_PollEvent(&Event))
        {
            dispatchEvent(&Event);
        }

        tick();
        // no need to call this too often...
        //render();
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

void SDLApp::tick()
{
    auto it = wnds.begin();
    while (it != wnds.end())
    {
        (*it)->onTick();
        ++it;
    }
}

void SDLApp::render()
{
    auto it = wnds.begin();
    while (it != wnds.end())
    {
        (*it)->onTick();
        ++it;
    }
}

void SDLApp::clean()
{
    running = false;
    auto it = wnds.begin();
    while (it != wnds.end())
    {
        delete* it;
        ++it;
    }
    wnds.clear();

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
    if (SDL_GetWindowFromID(ev.windowID) == nullptr)
    {
        SDL_Log("Widnow Event with an invalid windowID!");
        return;
    }

    switch (ev.event)
    {
    case SDL_WINDOWEVENT_SHOWN:        getWnd(ev.windowID)->onShown();      break;
    case SDL_WINDOWEVENT_HIDDEN:       getWnd(ev.windowID)->onHidden();     break;
    case SDL_WINDOWEVENT_EXPOSED:      getWnd(ev.windowID)->onExposed();    break;
    case SDL_WINDOWEVENT_MOVED:        getWnd(ev.windowID)->onMoved(ev.data1, ev.data2);       break;
    case SDL_WINDOWEVENT_RESIZED:      getWnd(ev.windowID)->onResized(ev.data1, ev.data2);     break;
    case SDL_WINDOWEVENT_SIZE_CHANGED: getWnd(ev.windowID)->onSizeChanged(ev.data1, ev.data2); break;
    case SDL_WINDOWEVENT_MINIMIZED:    getWnd(ev.windowID)->onMinimized();           break;
    case SDL_WINDOWEVENT_MAXIMIZED:    getWnd(ev.windowID)->onMaximized();           break;
    case SDL_WINDOWEVENT_RESTORED:     getWnd(ev.windowID)->onRestored();            break;
    case SDL_WINDOWEVENT_ENTER:        getWnd(ev.windowID)->onMouseEnter();          break;
    case SDL_WINDOWEVENT_LEAVE:        getWnd(ev.windowID)->onMouseLeave();          break;
    case SDL_WINDOWEVENT_FOCUS_GAINED: getWnd(ev.windowID)->onKeyboardFocusGained(); break;
    case SDL_WINDOWEVENT_FOCUS_LOST:   getWnd(ev.windowID)->onKeyboardFocusLost();   break;
    case SDL_WINDOWEVENT_CLOSE:        getWnd(ev.windowID)->onClose();               break;
#if SDL_VERSION_ATLEAST(2,0,5)
    case SDL_WINDOWEVENT_TAKE_FOCUS:   getWnd(ev.windowID)->onTakeFocus();           break;
    case SDL_WINDOWEVENT_HIT_TEST:     getWnd(ev.windowID)->onHitTest();             break;
#endif

#if SDL_VERSION_ATLEAST(2,0,18)
    case SDL_WINDOWEVENT_ICCPROF_CHANGED: getWnd(ev.windowID)->onIccprofChanged();   break;
    case SDL_WINDOWEVENT_DISPLAY_CHANGED: getWnd(ev.windowID)->onDisplayChanged(ev.data1); break;
#endif
    default: SDL_Log("Unhandled SDL_WINDOWEVENT for window : %d", ev.windowID);       break;
    }
}

void SDLApp::dispatchKeyDownEvent(const SDL_KeyboardEvent& ev)
{
    if (onKeyDown(ev.keysym, ev.repeat) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onKeyDown(ev.keysym, ev.repeat);
    }
}

void SDLApp::dispatchKeyUpEvent(const SDL_KeyboardEvent& ev)
{
    if (onKeyUp(ev.keysym, ev.repeat) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onKeyUp(ev.keysym, ev.repeat);
    }
}

void SDLApp::dispatchTextEditingEvent(const SDL_TextEditingEvent& ev)
{
    if (onTextEditing(ev.text, ev.start, ev.length) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onTextEditing(ev.text, ev.start, ev.length);
    }
}

void SDLApp::dispatchTextInputEvent(const SDL_TextInputEvent& ev)
{
    if (onTextInput(ev.text) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onTextInput(ev.text);
    }
}

//https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent
void SDLApp::dispatchMouseMotionEvent(const SDL_MouseMotionEvent& ev)
{
    if (onMouseMove(ev.which, ev.state, ev.x, ev.y, ev.xrel, ev.yrel) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onMouseMove(ev.which, ev.state, ev.x, ev.y, ev.xrel, ev.yrel);
    }
}

void SDLApp::dispatchMouseButtonDownEvent(const SDL_MouseButtonEvent& ev)
{
    if (onMouseButtonDown(ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onMouseButtonDown(ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y);
    }
}

void SDLApp::dispatchMouseButtonUpEvent(const SDL_MouseButtonEvent& ev)
{
    if (onMouseButtonUp(ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onMouseButtonUp(ev.which, ev.button, ev.state, ev.clicks, ev.x, ev.y);
    }
}

void SDLApp::dispatchMouseWheelEvent(const SDL_MouseWheelEvent& ev)
{
    if (onMouseWheel(ev.which, ev.x, ev.y, ev.direction, ev.preciseX, ev.preciseY) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onMouseWheel(ev.which, ev.x, ev.y, ev.direction, ev.preciseX, ev.preciseY);
    }
}

void SDLApp::dispatchUserEvent(const SDL_UserEvent& ev)
{
    if (onUserEvent(ev.code, ev.data1, ev.data2) == false &&
        SDL_GetWindowFromID(ev.windowID) != nullptr)
    {
        getWnd(ev.windowID)->onUserEvent(ev.code, ev.data1, ev.data2);
    }
}

void SDLApp::onQuit()
{
    running = false;
}

SDLWindow* SDLApp::getWnd(Uint32 wndId)
{
    // an simple speedup to avoid some std::map::find() calling..
    if (lastActiveWnd && lastActiveWnd->getWndId() == wndId)
    {
        return lastActiveWnd;
    }

    SDL_Window* wnd = SDL_GetWindowFromID(wndId);
    lastActiveWnd = (SDLWindow*)SDL_GetWindowData(wnd, "user_class");

    if (lastActiveWnd == nullptr)
    {
        lastActiveWnd = new SDLWindow(wnd);
    }
    return lastActiveWnd;
}

void SDLApp::insertWindow(SDLWindow* wnd)
{
    bool found = false;
    if (wnd != nullptr)
    {
        auto it = wnds.begin();
        while (it != wnds.end())
        {
            if (wnd == *it)
            {
                SDL_Log("window %d is already in wnds.\n", wnd->getWndId());
                found = true;
                break;
            }
            ++it;
        }
        if (found == false)
        {
            wnds.push_back(wnd);
        }
    }
}

void SDLApp::removeWindow(SDLWindow* wnd)
{
    if (wnd != nullptr)
    {
        auto it = wnds.begin();
        while (it != wnds.end())
        {
            if (*it == wnd)
            {
                wnds.erase(it);
                break;
            }
            ++it;
        }
    }

    // if now window is alive,the appliction is going to quite.
    if (wnds.size() == 0)
    {
        running = false;
    }
}

