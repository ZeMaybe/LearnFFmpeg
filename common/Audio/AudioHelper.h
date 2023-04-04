
#pragma once

extern "C"
{
#include "libswresample/swresample.h"
#include "SDL.h"
}

class FFmpegDecoder;
class AudioHelper
{
public:
    AudioHelper(FFmpegDecoder* wnd);
    virtual ~AudioHelper();

    void start();
    void pause();

    int getChannelNums()const;
    int getDeviceSampleRate()const;
    AVSampleFormat getDeviceSampleFmt()const;

    void pushAudioData(Uint8* stream,int len);
    static void audio_callback_func(void* userdata, Uint8* stream, int len);
    static AVSampleFormat sdlAudioFmt2FFmpegSampleFmt(SDL_AudioFormat fmt);
protected:
    char* defaultDevname = nullptr;
    SDL_AudioSpec defaultDevSpec;
    SDL_AudioDeviceID devId = 0;

    FFmpegDecoder* decoder = nullptr;
    AVFrame* frame = nullptr;
    int avaliableDataLen = 0;
    int avaliableDataPos = 0;
};