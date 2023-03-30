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

//#include "cuda.h"
//#include "cuda_runtime.h"
//#include "ColorSpace.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h""

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

#if false
            // test code..
            if (frame->format == AV_PIX_FMT_CUDA)
            {
                AVFrame* tmpFrame = av_frame_alloc();
                if (av_hwframe_transfer_data(tmpFrame, frame, 0) < 0)
                {
                    fprintf(stderr, "Error transferring the data to system memory\n");
                }

                static FILE* output_file = nullptr;
                if (output_file == nullptr)
                {
                    char name[64] = { 0 };
                    sprintf_s(name, "out%dx%d.y", tmpFrame->width, tmpFrame->height);
                    output_file = fopen(name, "w+b");
                }

                size_t size = tmpFrame->linesize[0] * tmpFrame->height;
                if (fwrite(tmpFrame->data[0], 1, size, output_file) < 0) {
                    fprintf(stderr, "Failed to dump raw data.\n");
                }
                av_frame_free(&tmpFrame);
            }
#endif

#if false
            static FILE* output_file = nullptr;
            if (output_file == nullptr)
            {
                char name[64] = { 0 };
                sprintf_s(name, "cuda_out_%dx%d.y", frame->linesize[0], frame->height);
                output_file = fopen(name, "w+b");
            }

            char* buffer = new char[frame->linesize[0] * frame->height];
            cudaMemcpy(buffer, frame->data[0], frame->linesize[0] * frame->height, cudaMemcpyDeviceToHost);

            fwrite(buffer, 1, frame->linesize[0] * frame->height, output_file);
            delete[] buffer;
#endif

#if false
            void* buff = nullptr;
            cudaMalloc(&buff, frame->width * frame->height * 4);
            //Nv12ToColor32<RGBA32>(frame->buf[0]->data, frame->linesize[0], (uint8_t*)buff, frame->width * 4, frame->height, (int)frame->colorspace);

            Nv12ToColor32<BGRA32>(frame->data[0], frame->linesize[0], (uint8_t*)buff, frame->width * 4, frame->width, frame->height, (int)frame->colorspace);

            static int count = 0;
            char bmpName[32] = { 0 };
            sprintf_s(bmpName, "bmp/%d.bmp", count++);
            char* tmpData = new char[frame->width * frame->height * 4];
            cudaMemcpy(tmpData, buff, frame->width * frame->height * 4, cudaMemcpyDeviceToHost);
            stbi_write_bmp(bmpName, frame->width, frame->height, 4, tmpData);
            //stbi_write_png(bmpName, frame->width, frame->height, 4, tmpData, 4);
            delete[]tmpData;
            cudaFree(buff);
#endif

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
    for (int i = 0; i < 1; ++i)
    {
        char title[128] = { 0 };
        sprintf_s(title, "test window %d",i);
        TestWindow* wnd = new TestWindow(title,"bbb_sunflower_1080p_60fps_normal.mp4", 800, 600);
    }
    return app.exec();
}
