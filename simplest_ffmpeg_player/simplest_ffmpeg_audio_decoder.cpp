
#include "SDLApp.h"
#include "SDLWindow.h"
#include "FFmpegDecoder.h"
#include "AudioHelper.h"

class TestWindow : public SDLWindow
{
public:
    TestWindow(const char* wndTitle, const char* filePath, int w, int h)
        :SDLWindow(wndTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
    {
        decoder = new FFmpegDecoder(filePath, true, false);
        audioHelper = new AudioHelper(decoder);

        decoder->setAudioResample(true,
            av_get_default_channel_layout(audioHelper->getChannelNums()),
            audioHelper->getDeviceSampleFmt(),
            audioHelper->getDeviceSampleRate());
        t = sdlApp->getRunningTime();
    }

    virtual ~TestWindow()
    {
        delete decoder;
        delete audioHelper;
    }

    virtual void onTick()override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        static bool flag = false;
        if (flag == false)
        {
            flag = true;
            audioHelper->start();
        }
    }
protected:
    FFmpegDecoder* decoder = nullptr;
    AudioHelper* audioHelper = nullptr;
    FILE* outputFile;
};

int main(int argc, char* argv[])
{
    SDLApp app(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    //TestWindow* wnd = new TestWindow("test audio", "music.mp3", 800, 640);
    TestWindow* wnd = new TestWindow("test audio", "bbb_sunflower_1080p_60fps_normal.mp4", 800, 640);
    return app.exec();
}
