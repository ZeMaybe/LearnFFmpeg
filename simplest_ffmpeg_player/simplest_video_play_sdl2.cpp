/**
 * 最简单的SDL2播放视频的例子（SDL2播放RGB/YUV）
 * Simplest Video Play SDL2 (SDL2 play RGB/YUV)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序使用SDL2播放RGB/YUV视频像素数据。SDL实际上是对底层绘图
 * API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层
 * API。
 *
 * 函数调用步骤如下:
 *
 * [初始化]
 * SDL_Init(): 初始化SDL。
 * SDL_CreateWindow(): 创建窗口（Window）。
 * SDL_CreateRenderer(): 基于窗口创建渲染器（Render）。
 * SDL_CreateTexture(): 创建纹理（Texture）。
 *
 * [循环渲染数据]
 * SDL_UpdateTexture(): 设置纹理的数据。
 * SDL_RenderCopy(): 纹理复制给渲染器。
 * SDL_RenderPresent(): 显示。
 *
 * This software plays RGB/YUV raw video data using SDL2.
 * SDL is a wrapper of low-level API (Direct3D, OpenGL).
 * Use SDL is much easier than directly call these low-level API.
 *
 * The process is shown as follows:
 *
 * [Init]
 * SDL_Init(): Init SDL.
 * SDL_CreateWindow(): Create a Window.
 * SDL_CreateRenderer(): Create a Render.
 * SDL_CreateTexture(): Create a Texture.
 *
 * [Loop to Render data]
 * SDL_UpdateTexture(): Set Texture's data.
 * SDL_RenderCopy(): Copy Texture to Render.
 * SDL_RenderPresent(): Show.
 */

#include "SDLApp.h"
#include "SDLWindow.h"
#include <mutex>

class SimpleWindow :public SDLWindow
{
public:
    SimpleWindow(const char* path, int w, int h)
        :SDLWindow("simple window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
    {
        createTexture(SDL_PIXELFORMAT_IYUV, pixelWidth, pixelHeight);

        inputFile = fopen(path, "rb+");
        SDL_assert(inputFile != nullptr);
        buffLen = pixelWidth * pixelHeight * bpp / 8;
        pixelBuff = new unsigned char[buffLen];

        t = sdlApp->getRunningTime();
        subThread = SDL_CreateThread(SimpleWindow::subThread_func, nullptr, this);
    }

    virtual ~SimpleWindow()
    {
        {
            std::lock_guard g(buffMutex);
            buffFlag = true;
            subThreadRunning = false;
            if (inputFile)
                fclose(inputFile);
            if (pixelBuff)
                delete[] pixelBuff;
        }

        int tmp;
        SDL_WaitThread(subThread, &tmp);
    }

    static int subThread_func(void* data)
    {
        SimpleWindow* simpleWnd = (SimpleWindow*)data;

        while (true)
        {
            std::lock_guard g(simpleWnd->buffMutex);
            if (simpleWnd->buffFlag == false)
            {
                //SDL_Log("before read:%lld", sdlApp->getRunningTime());
                if (fread(simpleWnd->pixelBuff, 1, simpleWnd->buffLen, simpleWnd->inputFile) != simpleWnd->buffLen)
                {
                    fseek(simpleWnd->inputFile, 0, SEEK_SET);
                    fread(simpleWnd->pixelBuff, 1, simpleWnd->buffLen, simpleWnd->inputFile);
                }
                simpleWnd->buffFlag = true;
                //SDL_Log("after  read:%lld", sdlApp->getRunningTime());
            }
            if (simpleWnd->subThreadRunning == false)
                break;
        }
        return 0;
    }

    virtual void onTick()override
    {
        if (sdlApp->getRunningTime() - t >= 16)
        {
            std::lock_guard g(buffMutex);
            if (buffFlag == true)
            {
                SDL_UpdateTexture(texture, nullptr, pixelBuff, pixelWidth);
                t = sdlApp->getRunningTime();
                buffFlag = false;
                //SDL_Log("          t:%lld", t);
            }
        }
    }

protected:
    FILE* inputFile = nullptr;

    int pixelWidth = 1920;
    int pixelHeight = 1080;
    const int bpp = 12;                   // bits per pixel ?
    int buffLen = 0;
    unsigned char* pixelBuff = nullptr;
    std::atomic<bool> buffFlag = false;
    bool subThreadRunning = true;
    SDL_Thread* subThread = nullptr;
    std::mutex buffMutex;

    long long t = 0;
};

int main(int argc, char* argv[])
{
    SDLApp app(SDL_INIT_EVERYTHING);
    SimpleWindow* wnd = new SimpleWindow("output.yuv", 800.0 * 1920.0 / 1080.0, 800);
    //SimpleWindow* wnd1 = new SimpleWindow("output.yuv", 800.0 * 1920.0 / 1080.0, 800);
    return app.exec();
}
