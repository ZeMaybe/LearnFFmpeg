
#include "AudioHelper.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}
#include "FFmpegDecoder.h"
#include "SDLApp.h"

AudioHelper::AudioHelper(FFmpegDecoder* d)
    :decoder(d)
{
    if (SDL_GetDefaultAudioInfo(&defaultDevname, &defaultDevSpec, 0) != 0)
    {
        SDL_Log("couldn't get default audio playback device.\n");
    }
    else
    {
        SDL_Log("default audio device:%s.\n", defaultDevname);
        defaultDevSpec.format |= SDL_AUDIO_MASK_SIGNED;
#if AV_HAVE_BIGENDIAN
        defaultDevSpec.format |= SDL_AUDIO_MASK_ENDIAN;
#else
        defaultDevSpec.format &= (~SDL_AUDIO_MASK_ENDIAN);
#endif
        defaultDevSpec.samples = 512;
        defaultDevSpec.userdata = this;
        defaultDevSpec.callback = AudioHelper::audio_callback_func;
    }
}

AudioHelper::~AudioHelper()
{
    if (devId != 0)
    {
        SDL_CloseAudioDevice(devId);
        devId = 0;
    }

    if (defaultDevname != nullptr)
    {
        SDL_free(defaultDevname);
    }
}

void AudioHelper::start()
{
    if (devId <= 0)
    {
        devId = SDL_OpenAudioDevice(defaultDevname, 0, &defaultDevSpec, nullptr, 0);
        if (devId == 0)
        {
            SDL_Log("couldn't open sdl audio device.\n");
            return;
        }
    }
    if (devId > 0)
    {
        SDL_PauseAudioDevice(devId, 0);
    }
}

void AudioHelper::pause()
{
    if (devId > 0)
    {
        SDL_PauseAudioDevice(devId, 1);
    }
}

int AudioHelper::getDeviceSampleRate() const
{
    return defaultDevSpec.freq;
}

AVSampleFormat AudioHelper::getDeviceSampleFmt() const
{
    return sdlAudioFmt2FFmpegSampleFmt(defaultDevSpec.format);
}

int AudioHelper::getChannelNums() const
{
    return defaultDevSpec.channels;
}

AVSampleFormat AudioHelper::sdlAudioFmt2FFmpegSampleFmt(SDL_AudioFormat fmt)
{
    switch (fmt)
    {
    case AUDIO_U8:
    case AUDIO_S8:
        return AV_SAMPLE_FMT_U8;
    case AUDIO_U16LSB:
    case AUDIO_S16LSB:
    case AUDIO_U16MSB:
    case AUDIO_S16MSB:
        return AV_SAMPLE_FMT_S16;
    case AUDIO_S32LSB:
    case AUDIO_S32MSB:
        return AV_SAMPLE_FMT_S32;
    case AUDIO_F32LSB:
    case AUDIO_F32MSB:
        return AV_SAMPLE_FMT_FLT;

    default:
        return AV_SAMPLE_FMT_NONE;
    }
}

void AudioHelper::audio_callback_func(void* userdata, Uint8* stream, int len)
{
    if (userdata == nullptr)
        return;

    AudioHelper* helper = (AudioHelper*)userdata;
    helper->pushAudioData(stream, len);
}

void AudioHelper::pushAudioData(Uint8* stream, int len)
{
    SDL_memset(stream, 0, len);
    int dstPos = 0;

    while (len > 0)
    {
        if (frame == nullptr)
        {
            avaliableDataLen = 0;
            avaliableDataPos = 0;
            bool running;
            frame = decoder->getAudioFrame(running);
            if (frame != nullptr)
            {
                avaliableDataLen = frame->nb_samples * frame->ch_layout.nb_channels * av_get_bytes_per_sample((AVSampleFormat)frame->format);
                avaliableDataPos = 0;
            }
            if (running == false)
            {
                return;
            }
        }

        if (frame != nullptr)
        {
            int tmpLen = avaliableDataLen <= len ? avaliableDataLen : len;
            SDL_MixAudioFormat(&(stream[dstPos]), &(frame->data[0][avaliableDataPos]), defaultDevSpec.format, tmpLen, SDL_MIX_MAXVOLUME);
            avaliableDataLen -= tmpLen;
            avaliableDataPos += tmpLen;
            dstPos += tmpLen;
            len -= tmpLen;

            if (avaliableDataLen <= 0)
            {
                av_frame_free(&frame);
                frame = nullptr;
            }
        }
    }
}
