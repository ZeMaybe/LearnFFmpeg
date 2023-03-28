/**
 * ��򵥵Ļ���FFmpeg����Ƶ������ 2
 * Simplest FFmpeg Player 2
 *
 * ������ Lei Xiaohua
 * leixiaohua1020@126.com
 * �й���ý��ѧ/���ֵ��Ӽ���
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * ��2��ʹ��SDL2.0ȡ���˵�һ���е�SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * ������ʵ������Ƶ�ļ��Ľ������ʾ(֧��HEVC��H.264��MPEG2��)��
 * ����򵥵�FFmpeg��Ƶ���뷽��Ľ̡̳�
 * ͨ��ѧϰ�����ӿ����˽�FFmpeg�Ľ������̡�
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 */

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
  //Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"

#include "GL/glew.h"
#include "GL/wglew.h"
#include "SDL.h"
};
#else
 //Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

#include "SDLApp.h"
#include "SDLWindow.h"
#include "FFmpegDecoder.h"
#include "convert.h"
#include "SDLRenderHelper.h"
#include "OpenGLRenderHelper.h"

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

class TestWindow : public SDLWindow
{
public:
    TestWindow(const char* videoPath, int w, int h)
        :SDLWindow("simple window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
    {
        decoder = new FFmpegDecoder(videoPath, wndId);
        //renderHelper = new SDLRenderHelper(wnd);
        renderHelper = new OpenGLRenderHelper(wnd);
        frame = av_frame_alloc();
        t = sdlApp->getRunningTime();
    }

    virtual ~TestWindow()
    {
        av_frame_free(&frame);
        delete decoder;
    }

    virtual void onTick()override
    {
        if (sdlApp->getRunningTime() - t > 1000.0 / decoder->getFrameRate())
        {
            t = sdlApp->getRunningTime();
            decoder->receiveVideoFrame(frame);
            //SDL_Log("received on frame:%d", wndId);
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
    TestWindow* wnd1 = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    TestWindow* wnd2 = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    TestWindow* wnd3 = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    TestWindow* wnd4 = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    TestWindow* wnd5 = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    TestWindow* wnd6 = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    TestWindow* wnd7 = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    return app.exec();
}
