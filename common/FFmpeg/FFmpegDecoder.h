
#pragma once


extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}
#include <mutex>
#include <deque>

class FFmpegHwDecoderHelper;
class FFmpegDecoder
{
public:
    FFmpegDecoder();
    FFmpegDecoder(const char* filePath = nullptr, bool pumpAudio = false,bool pumpVideo = true,bool hwAccel = true,const char* hwName = "cuda");
    virtual ~FFmpegDecoder();

    void setId(int id) { wndId = id; }

    int getId()const { return wndId; }
    bool getPumpAudio()const { return pumpAudio; }
    bool getPumpVideo()const { return pumpVideo; }
    bool getHwAccel()const { return hwAccel; }

    // these 'set functions' must be called before 'loadFile'
    void setPumpAudio(bool pumpAudio);
    void setPumpVideo(bool pumpVideo);
    void setHwAccel(bool hwAccel, const char* hwName);
    const char* getHwAccelName()const { return hwName; }
    bool loadFile(const char* filePath, bool pumpAudio = false, bool pumpVideo = true,bool hwAccel = true,const char* hwName = "cuda");

    void setAudioResample(bool enable = true, uint64_t outChannelLayout = AV_CH_LAYOUT_STEREO, AVSampleFormat outFormat = AV_SAMPLE_FMT_S16, int outSampleRate = 44100);

    void pause();
    void resume();
    AVFrame* getVideoFrame();
    AVFrame* getAudioFrame();
    AVFrame* getNextVideoFrame();
    AVFrame* getNextAudioFrame();

    AVPixelFormat getOutputFramePixelFormat()const { return outputFramePixelFormat; }
    void setOutputFramePixelFormat(AVPixelFormat newFmt);

    // video
    int getPixelWidth()const;
    int getPixelHeight()const;
    void getPixelSize(int* w, int* h)const;
    double getFrameRate()const;

    // audio
    AVSampleFormat getAudioSampleFormat()const;

protected:
    AVFormatContext* formatCtx = nullptr;
    void resetInputFormat();

    bool loadCodec(const AVCodec*& codec, AVCodecContext*& ctx, int& index, AVMediaType t);

    // video
    bool pumpVideo = true;
    bool hwAccel = true;
    const char* hwName = "cuda";
    friend class FFmpegHwDecoderHelper;
    FFmpegHwDecoderHelper* helper = nullptr;

    AVCodecContext* videoCodecCtx = nullptr;
    const AVCodec* videoCodec = nullptr;
    int videoIndex = -1;
    AVPixelFormat outputFramePixelFormat = AV_PIX_FMT_NONE;
    void resetVideoCodec();
    bool loadVideoCodec();

    std::deque<AVFrame*> videoQueue;
    std::atomic<bool> videoFull = false;
    std::mutex videoMutex;
    int videoQueueSize = 2;
    void resetVideoData();

    // audio
    bool pumpAudio = false;
    AVCodecContext* audioCodecCtx = nullptr;
    const AVCodec* audioCodec = nullptr;
    int audioIndex = -1;
    void resetAudioCodec();
    bool loadAudioCodec();

    std::deque<AVFrame*> audioQueue;
    std::atomic<bool> audioFull = false;
    std::mutex audioMutex;
    int audioQueueSize = 4;
    void resetAudioData();

    bool swrEnable = true;
    uint64_t outputChanelLayout = AV_CH_LAYOUT_STEREO;
    AVSampleFormat outputSampleFormat = AV_SAMPLE_FMT_S16;
    int outputSampleRate = 44100;

    int outputChannelNums = 0;
    int inputChannelNums = 0;
    SwrContext* swrCtx = nullptr;
    void resetSwrCtx();
    bool loadSwrCtx();
    AVFrame* convertAudioFrame(AVFrame* frame);

    // thread
    std::atomic<bool> running = false;
    std::atomic<bool> pauseDecoding = false;
    std::thread decoderThread;
    void stopDecoderThread();
    void run();
    bool sendPacket(AVPacket* p, AVCodecContext*& ctx, std::deque<AVFrame*>& queue, std::atomic<bool>& full, std::mutex& mutex, const int& maxSize);
    bool sendVideoPacket(AVPacket* p);
    bool sendAudioPacket(AVPacket* p);
    AVFrame* getFrame(std::mutex& mutex,std::atomic<bool>& fullFlag,int& maxSize,std::deque<AVFrame*>& queue);

    AVPacket* packet = nullptr;

    int wndId = 0;
    void clear();
};
