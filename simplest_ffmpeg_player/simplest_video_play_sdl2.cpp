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

class SimpleWindow :public SDLWindow
{
public:
    SimpleWindow(const char* path,int w, int h)
        :SDLWindow("simple window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE) 
    {
        createTexture(SDL_PIXELFORMAT_IYUV, pixelWidth, pixelHeight);

        inputFile = fopen(path, "rb+");
        SDL_assert(inputFile != nullptr);
        buffLen = pixelWidth * pixelHeight * bpp / 8;
        pixelBuff = new unsigned char[buffLen];
    }

    virtual ~SimpleWindow()
    {
        if (inputFile)
            fclose(inputFile);
        if (pixelBuff)
            delete[] pixelBuff;
    }

    virtual void onLoop()override
    {
        if (fread(pixelBuff, 1, buffLen, inputFile) != buffLen)
        {
            fseek(inputFile, 0, SEEK_SET);
            fread(pixelBuff, 1, buffLen,inputFile);
        }

        SDL_UpdateTexture(texture, nullptr, pixelBuff, pixelWidth);
    }

protected:
    FILE* inputFile = nullptr;

    int pixelWidth = 1920;
    int pixelHeight = 1080;
    const int bpp = 12;                   // bits per pixel ?
    int buffLen = 0;
    unsigned char* pixelBuff = nullptr;
};

int main(int argc, char* argv[])
{
    SDLApp app(SDL_INIT_EVERYTHING);
    SimpleWindow* wnd = new SimpleWindow("output.yuv", 800, 600);
    return app.exec();
}
