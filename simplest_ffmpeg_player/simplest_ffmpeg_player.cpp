/**
 * 最简单的基于FFmpeg的视频播放器 2
 * Simplest FFmpeg Player 2
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 第2版使用SDL2.0取代了第一版中的SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
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

class TestWindow : public SDLWindow
{
public:
    TestWindow(const char* videoPath, int w, int h)
        :SDLWindow("simple window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
    {
        decoder.loadFile(videoPath);
        frame = av_frame_alloc();
        t = sdlApp->getRunningTime();

        //createTexture(pixelFmtFFmpegToSDL(decoder.getOutputFramePixelFormat()), decoder.getPixelWidth(), decoder.getPixelHeight());
    }

    virtual ~TestWindow()
    {
        av_frame_free(&frame);
    }

    virtual void onTick()override
    {
        if (sdlApp->getRunningTime() - t > 1000.0/decoder.getFrameRate())
        {
            decoder.receiveVideoFrame(frame);

            if (texture == nullptr)
            {
                createTexture(pixelFmtFFmpegToSDL((AVPixelFormat)frame->format), frame->width, frame->height);
                t = sdlApp->getRunningTime();
            }

            SDL_UpdateYUVTexture(texture, nullptr, frame->data[0], frame->linesize[0],
                frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
            t = sdlApp->getRunningTime();
        }
    }


protected:
    FFmpegDecoder decoder;
    AVFrame* frame = nullptr;
    long long t = 0;
};


int main(int argc, char* argv[])
{
    SDLApp app(SDL_INIT_EVERYTHING);
    TestWindow* wnd = new TestWindow("bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    return app.exec();
}
