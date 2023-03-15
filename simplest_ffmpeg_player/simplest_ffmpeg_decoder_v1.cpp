/**
 * 最简单的基于FFmpeg的视频解码器
 * Simplest FFmpeg Decoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * 本程序实现了视频文件解码为YUV数据。它使用了libavcodec和
 * libavformat。是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest decoder based on FFmpeg.
 * It decodes video to YUV pixel data.
 * It uses libavcodec and libavformat.
 * Suitable for beginner of FFmpeg.
 *
 */

#include <iostream>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
 //Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
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
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

char* get_input_file_path(int argc, char* argv[])
{
    static char default_in_path[] = "input.mp4";

    if (argc >= 2)
    {
        return argv[1];
    }
    else
    {
        return default_in_path;
    }
}

char* get_output_file_path(int argc, char* argv[])
{
    static char default_out_path[] = "output.yuv";
    if (argc >= 3)
    {
        return argv[2];
    }
    else
    {
        return default_out_path;
    }
}

int main(int argc, char* argv[])
{
    char* input_path = get_input_file_path(argc, argv);
    AVFormatContext* fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, input_path, nullptr, nullptr);
    if (ret < 0)
    {
        std::cout << "Can't open input file:" << input_path << std::endl;
        return ret;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0)
    {
        std::cout << "Can't find stream information.\n";
        return ret;
    }

    const AVCodec* video_codec = nullptr;
    int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0);
    if (video_stream_index < 0)
    {
        std::cout << "Can't find video stream.\n";
        return video_stream_index;
    }

    AVCodecContext* video_codec_ctx = avcodec_alloc_context3(video_codec);
    if (video_codec_ctx == nullptr)
    {
        std::cout << "Can't alloc codec context.\n";
        return AVERROR(ENOMEM);
    }
    ret = avcodec_parameters_to_context(video_codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);
    if (ret < 0)
    {
        std::cout << "avcodec_parameters_to_context failed.\n";
        return ret;
    }

    ret = avcodec_open2(video_codec_ctx, video_codec, nullptr);
    if (ret < 0)
    {
        std::cout << "Can't open video decoder.\n";
        return ret;
    }

    char* output_path = get_output_file_path(argc, argv);
    FILE* output_file = fopen(output_path, "wb+");

    AVFrame* yuv_frame = av_frame_alloc();
    unsigned char* yuv_buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video_codec_ctx->width, video_codec_ctx->height, 1));
    av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize, yuv_buffer, AV_PIX_FMT_YUV420P, video_codec_ctx->width, video_codec_ctx->height, 1);

    SwsContext* sws_ctx = sws_getContext(video_codec_ctx->width, video_codec_ctx->height, video_codec_ctx->pix_fmt, video_codec_ctx->width, video_codec_ctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);


    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    bool flag = true;
    int len = 0;
    int count = 0;
    while (flag)
    {
        ret = av_read_frame(fmt_ctx, packet);
        if (ret < 0)
        {
            packet->data = nullptr;   // send an empty packet in the last. to clean the decodec buffer.
            packet->size = 0;
        }

        if (packet->stream_index != video_stream_index)
        {
            av_packet_unref(packet);
            continue;
        }

        ret = avcodec_send_packet(video_codec_ctx, packet);
        if (ret < 0)
        {
            std::cout << "Error while sending a packet to the decoder.\n";
            break;
        }

        while (ret >= 0)
        {
            ret = avcodec_receive_frame(video_codec_ctx, frame);
            if (ret == AVERROR(EAGAIN))
            {
                break;
            }
            else if (ret == AVERROR_EOF)
            {
                flag = false;
                break;
            }
            else if (ret < 0)
            {
                std::cout << "Error while receiving a frame from the decoder.\n";
                return -1;
            }

            sws_scale(sws_ctx, frame->data, frame->linesize, 0, video_codec_ctx->height, yuv_frame->data, yuv_frame->linesize);
            len = video_codec_ctx->width * video_codec_ctx->height;

            fwrite(yuv_frame->data[0], 1, len, output_file);
            fwrite(yuv_frame->data[1], 1, len / 4, output_file);
            fwrite(yuv_frame->data[2], 1, len / 4, output_file);

            fflush(output_file);
            std::cout << "Success to decode frame: " << ++count << std::endl;
        }

        av_packet_unref(packet);
    }

    fclose(output_file);

    av_free(yuv_buffer);
    av_frame_free(&frame);
    av_frame_free(&yuv_frame);
    av_packet_free(&packet);
    sws_freeContext(sws_ctx);

    avcodec_free_context(&video_codec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}

