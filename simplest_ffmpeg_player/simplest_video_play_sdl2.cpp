/**
 * ��򵥵�SDL2������Ƶ�����ӣ�SDL2����RGB/YUV��
 * Simplest Video Play SDL2 (SDL2 play RGB/YUV)
 *
 * ������ Lei Xiaohua
 * leixiaohua1020@126.com
 * �й���ý��ѧ/���ֵ��Ӽ���
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * ������ʹ��SDL2����RGB/YUV��Ƶ�������ݡ�SDLʵ�����ǶԵײ��ͼ
 * API��Direct3D��OpenGL���ķ�װ��ʹ���������Լ���ֱ�ӵ��õײ�
 * API��
 *
 * �������ò�������:
 *
 * [��ʼ��]
 * SDL_Init(): ��ʼ��SDL��
 * SDL_CreateWindow(): �������ڣ�Window����
 * SDL_CreateRenderer(): ���ڴ��ڴ�����Ⱦ����Render����
 * SDL_CreateTexture(): ��������Texture����
 *
 * [ѭ����Ⱦ����]
 * SDL_UpdateTexture(): ������������ݡ�
 * SDL_RenderCopy(): �����Ƹ���Ⱦ����
 * SDL_RenderPresent(): ��ʾ��
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
