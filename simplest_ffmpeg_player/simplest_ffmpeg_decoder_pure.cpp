/**
 * 最简单的基于FFmpeg的视频解码器（纯净版）
 * Simplest FFmpeg Decoder Pure
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * 本程序实现了视频码流(支持HEVC，H.264，MPEG2等)解码为YUV数据。
 * 它仅仅使用了libavcodec（而没有使用libavformat）。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest decoder based on FFmpeg.
 * It decode bitstreams to YUV pixel data.
 * It just use libavcodec (do not contains libavformat).
 * Suitable for beginner of FFmpeg.
 */

#include <iostream>
#include <cstdio>
#include "common.h"

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
 //Windows
extern "C"
{
#include "libavcodec/avcodec.h"
};
#else
 //Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif
#endif

//test different codec
#define TEST_H264  1
#define TEST_HEVC  0

int main(int argc, char* argv[])
{
    AVCodecID codec_id = AV_CODEC_ID_H264;
    const AVCodec* codec = avcodec_find_decoder(codec_id);
    if (codec == nullptr)
    {
        std::cout << "Can't find codec : " << codec_id << std::endl;
        return -1;
    }

    AVCodecParserContext* parser_ctx = av_parser_init(codec->id);
    if (parser_ctx == nullptr)
    {
        std::cout << "Parser not found.\n";
        return -1;
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (codec_ctx == nullptr)
    {
        std::cout << "Can't allocate video codec context.\n";
        return -1;
    }

    int ret = avcodec_open2(codec_ctx, codec, nullptr);
    if (ret < 0)
    {
        std::cout << "Can't not open codec.\n";
        return -1;
    }

    FILE* input_file = fopen(get_input_file_path(argc, argv), "rb");
    if (input_file == nullptr)
    {
        std::cout << "Can't open input file :" << get_input_file_path(argc, argv) << std::endl;
        return -1;
    }

    FILE* output_file = fopen(get_output_file_path(argc, argv), "wb");
    if (output_file == nullptr)
    {
        std::cout << "Can't open output file:" << get_output_file_path(argc, argv) << std::endl;
        return -1;
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    uint8_t in_buff[IN_BUFF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE] = { 0 };

    uint8_t* data = nullptr;
    int data_size = 0;
    do
    {
        data_size = fread(in_buff, 1, IN_BUFF_SIZE, input_file);
        if (ferror(input_file) != 0)   break;

        data = in_buff;
        while (data_size > 0)
        {
            ret = av_parser_parse2(parser_ctx, codec_ctx, &packet->data, &packet->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0)
            {
                std::cout << "Error while parsing.\n";
                return -1;
            }

            data += ret;
            data_size -= ret;

            if (packet->size > 0)
            {
                ret = avcodec_send_packet(codec_ctx, packet);
                if (ret < 0)
                {
                    char errorbuf[1024] = { 0 };
                    av_strerror(ret, errorbuf, sizeof(errorbuf));
                    std::cout << "Error sending a packet for decoding:" << errorbuf << std::endl;
                    return -1;
                }

                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(codec_ctx, frame);
                    if (ret == AVERROR(EAGAIN))
                    {
                        break;
                    }
                    else if (ret == AVERROR_EOF)
                    {
                        break;
                    }
                    else if (ret < 0)
                    {
                        std::cout << "Error during decoding.\n";
                        return -1;
                    }

                    // save frame.   Notes:linesize[0] and frame->width may be different.
                    for (int i = 0; i < frame->height; ++i)
                        fwrite(frame->data[0] + i * frame->linesize[0], 1, frame->width, output_file);
                    for (int i = 0; i < frame->height / 2; ++i)
                        fwrite(frame->data[1] + i * frame->linesize[1], 1, frame->width / 2, output_file);
                    for (int i = 0; i < frame->height / 2; ++i)
                        fwrite(frame->data[2] + i * frame->linesize[2], 1, frame->width / 2, output_file);

                    std::cout << "Sucess decode frame:" << frame->display_picture_number << std::endl;
                }

                av_packet_unref(packet);
            }
        }
    } while (feof(input_file) == 0);

    fclose(input_file);
    fclose(output_file);

    av_frame_free(&frame);
    av_packet_free(&packet);

    avcodec_free_context(&codec_ctx);
    av_parser_close(parser_ctx);

    return 0;
}

