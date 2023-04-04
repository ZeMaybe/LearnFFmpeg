
extern "C"
{
#include "SDL.h"
}
#include <stdio.h>
#include <atomic>
#include <chrono>

std::atomic<int> avaLen;
std::atomic<int> avaPos;
Uint8* data = nullptr;


auto t = std::chrono::system_clock::now();

void audio_call_back(void* user_data, Uint8* stream, int len)
{
    auto t1 = std::chrono::system_clock::now();
    auto d = t1 - t;
    auto d2 = std::chrono::duration_cast<std::chrono::milliseconds>(d);

    SDL_Log("call back %lld",d2.count());
    t = t1;

    SDL_memset(stream, 0, len);
    if (avaLen > 0)
    {
        len = avaLen > len ? len : avaLen;
        SDL_MixAudio(stream, &(data[avaPos]), len, SDL_MIX_MAXVOLUME);
        SDL_Log("mix audio : %d.\n",len);
        avaPos += len;
        avaLen -= len;
    }
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("sdl init failed.\n");
        return 0;
    }

    SDL_AudioSpec as;
    SDL_memset(&as, 0, sizeof(as));
    as.freq = 44100;
    as.format = AUDIO_S16;
    as.channels = 2;
    as.silence = 0;
    as.samples = 1024;
    as.callback = audio_call_back;

    //if (SDL_OpenAudioDevice(nullptr, 0, &as, nullptr, 0) <= 0)
    if(SDL_OpenAudio(&as,nullptr) < 0)
    {
        SDL_Log("couldn't open audio device.\n");
        return 0;
    }

    FILE* pcmFile = fopen("outsss.pcm", "r+b");
    if (pcmFile == nullptr)
    {
        SDL_Log("couldn't open pcm file:%s.\n","outsss.pcm");
        return 0;
    }
    
    int buffLen = as.samples * as.channels*2*2 ;
    Uint8* buff = new Uint8[buffLen];

    SDL_PauseAudio(0);

    while (true)
    {
        int tmp = fread(buff, 1, buffLen, pcmFile);
        data = buff;
        avaPos = 0;
        avaLen = tmp;

        while (avaLen > 0)
        {
            SDL_Delay(1);
        }
    }
    return 0;
}