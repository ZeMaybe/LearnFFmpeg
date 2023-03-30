

#include "SDLApp.h"
#include "SDLWindow.h"
#include "FFmpegDecoder.h"
//#include "convert.h"
#include "SDLRenderHelper.h"


extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}


class TestWindow : public SDLWindow
{
public:
    TestWindow(const char* wndTitle,const char* videoPath, int w, int h)
        :SDLWindow(wndTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
    {
        frame = av_frame_alloc();
        renderHelper = new SDLRenderHelper(wnd);
        //decoder = new FFmpegDecoder(videoPath, wndId,false);
        t = sdlApp->getRunningTime();
    }

    virtual ~TestWindow()
    {
        av_frame_free(&frame);
        delete decoder;
    }

    virtual void onTick()override
    {
        SDL_Log("onTick!");
        if (decoder == nullptr)
        {
            decoder = new FFmpegDecoder("bbb_sunflower_1080p_60fps_normal.mp4", wndId, false);
        }
        if (sdlApp->getRunningTime() - t > 1000.0 / decoder->getFrameRate())
        {
            t = sdlApp->getRunningTime();
            decoder->receiveVideoFrame(frame);

            renderHelper->update(frame);
            av_frame_unref(frame);
            onRender();
        }
    }

protected:
    FFmpegDecoder* decoder = nullptr;
    AVFrame* frame = nullptr;
};

int main(int argc, char* argv[])
{
    SDLApp app(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    for (int i = 0; i < 1; ++i)
    {
        char title[128] = { 0 };
        sprintf_s(title, "test player %d",i);
        TestWindow* wnd = new TestWindow(title,"bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    }
    return app.exec();
}

