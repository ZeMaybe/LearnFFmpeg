

#include "SDLApp.h"
#include "SDLWindow.h"
#include "FFmpegDecoder.h"
#include "FFmpegHwDecoderHelper.h"
#include "OpenGLRenderHelper.h"
#include "SDLRenderHelper.h"
#include "AudioHelper.h"

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

class TestWnd : public SDLWindow
{
public:
    TestWnd(const char* wndTitle, const char* filePath, int w, int h)
        :SDLWindow(wndTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
    {
        if (FFmpegHwDecoderHelper::isHwAccelerationAvailable("cuda"))
        {
            decoder = new FFmpegDecoder(filePath, false);
            renderHelper = new OpenGLRenderHelper(wnd);
        }
        else
        {
            decoder = new FFmpegDecoder(filePath, true, true, false);
            renderHelper = new SDLRenderHelper(wnd);
        }

        if (decoder->getPumpAudio() == true)
        {
            audioHelper = new AudioHelper(decoder);
            decoder->setAudioResample(true, av_get_default_channel_layout(audioHelper->getChannelNums()), audioHelper->getDeviceSampleFmt(), audioHelper->getDeviceSampleRate());
            audioHelper->start();
        }
        t = sdlApp->getRunningTime();
    }

    virtual ~TestWnd()
    {
        if (audioHelper != nullptr)
        {
            delete audioHelper;
        }
        if (decoder != nullptr)
        {
            delete decoder;
        }
    }

    virtual void onTick()override
    {
        if (sdlApp->getRunningTime() - t > 1000.0 / decoder->getFrameRate())
        {
            t = sdlApp->getRunningTime();

            bool running;
            auto tmp = decoder->getVideoFrame(running);
            if (tmp)
            {
                renderHelper->update(tmp);
                av_frame_free(&tmp);
            }
            onRender();
        }
    }

protected:
    FFmpegDecoder* decoder = nullptr;
    AudioHelper* audioHelper = nullptr;
};

int main(int argc, char* argv[])
{
    SDLApp app(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    for (int i = 0; i < 1; ++i)
    {
        char title[128] = { 0 };
        sprintf_s(title, "test window %d", i);
        TestWnd* wnd = new TestWnd(title, "bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    }

    return app.exec();
}