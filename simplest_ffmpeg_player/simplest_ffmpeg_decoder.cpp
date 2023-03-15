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



#include <stdio.h>

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


int main(int argc, char* argv[])
{
    AVFormatContext* pFormatCtx;
    int				i, videoindex;
    const AVCodec* pCodec;
    AVCodecParserContext* parser;
    AVCodecContext* pCodecCtx;
    AVCodecParameters* pCodecParams;
    AVFrame* pFrame, * pFrameYUV;
    unsigned char* out_buffer;
    AVPacket* packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext* img_convert_ctx;

    // change this to your file location.
    char filepath[] = "1.mp4";

    FILE* fp_yuv = fopen("output.yuv", "wb+");

    //av_register_all();                     // deprecated since ffmpeg 4.0
    //avformat_network_init();               // not recommand any more

    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    videoindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        //if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            pCodecParams = pFormatCtx->streams[i]->codecpar;
            break;
        }

    if (videoindex == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    //pCodecCtx=pFormatCtx->streams[videoindex]->codec;
    //pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoindex]->codecpar->codec_id);
    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }
    parser = av_parser_init(pCodec->id);
    if (parser == nullptr)
    {
        printf_s("Parser not found.\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == nullptr)
    {
        printf_s("Can't allocate video codec context.\n");
        return -1;
    }

    if (avcodec_parameters_to_context(pCodecCtx, pCodecParams) < 0)
    {
        printf_s("avcodec_parameters_to_context failed!\n");
        return -1;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    //out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));
    out_buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecParams->width, pCodecParams->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
        AV_PIX_FMT_YUV420P, pCodecParams->width, pCodecParams->height, 1);



    packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    //Output Info-----------------------------
    printf("--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);
    printf("-------------------------------------------------\n");
    img_convert_ctx = sws_getContext(pCodecParams->width, pCodecParams->height, (AVPixelFormat)pCodecParams->format,
        pCodecParams->width, pCodecParams->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    int count = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0)
    {
        if (packet->stream_index == videoindex) {
            //ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            //if(ret < 0){
            //	printf("Decode Error.\n");
            //	return -1;
            //}

            int ret = avcodec_send_packet(pCodecCtx, packet);
            if (ret < 0)
            {
                printf_s("Error sending a packet for decoding\n");
                return -1;
            }

            while (ret >= 0)
            {
                ret = avcodec_receive_frame(pCodecCtx, pFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if (ret < 0)
                {
                    printf_s("Error during decoding.\n");
                    return -1;
                }

                sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
                y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y 
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
                fflush(fp_yuv);
                ++count;
                printf("Succeed to decode 1 frame! %d\n",count);
            }
        }
    }

    //av_free_packet(packet);
    av_packet_free(&packet);
    //flush decoder
    //FIX: Flush Frames remained in Codec
    //while (1) {
    //	ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
    //	if (ret < 0)
    //		break;
    //	if (!got_picture)
    //		break;
    //	sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
    //		pFrameYUV->data, pFrameYUV->linesize);

    //	int y_size=pCodecCtx->width*pCodecCtx->height;  
    //	fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y 
    //	fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
    //	fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V

    //	printf("Flush Decoder: Succeed to decode 1 frame!\n");
    //}

    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    av_parser_close(parser);

    return 0;
}

