/*
 * Copyright (c) 2017 Jun Zhao
 * Copyright (c) 2017 Kaixuan Liu
 *
 * HW Acceleration API (video decoding) decode sample
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 /**
  * @file HW-accelerated decoding API usage.example
  * @example hw_decode.c
  *
  * Perform HW-accelerated decoding with output frames from HW video
  * surfaces.
  */

#include <stdio.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
}

static AVBufferRef* hw_device_ctx = NULL;
static enum AVPixelFormat hw_pix_fmt;
static FILE* output_file = NULL;

// init ctx->hw_device_ctx
static int hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
        NULL, NULL, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}

// callback function,and pix_fmts is an array terminated with AV_PIX_FMT_NONE
static enum AVPixelFormat get_hw_format(AVCodecContext* ctx,
    const enum AVPixelFormat* pix_fmts)
{
    const enum AVPixelFormat* p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

static int decode_write(AVCodecContext* codecCtx, AVPacket* packet)
{
    AVFrame* frame = NULL, * sw_frame = NULL;
    AVFrame* tmp_frame = NULL;
    uint8_t* buffer = NULL;
    int size;
    int ret = 0;

    ret = avcodec_send_packet(codecCtx, packet);
    if (ret < 0) {
        fprintf(stderr, "Error during decoding\n");
        return ret;
    }

    while (1) {
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
            fprintf(stderr, "Can not alloc frame\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = avcodec_receive_frame(codecCtx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error while decoding\n");
            goto fail;
        }

        if (frame->format == hw_pix_fmt) {
            /* retrieve data from GPU to CPU */
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                fprintf(stderr, "Error transferring the data to system memory\n");
                goto fail;
            }
            tmp_frame = sw_frame;
        }
        else
            tmp_frame = frame;

        size = av_image_get_buffer_size((AVPixelFormat)tmp_frame->format, tmp_frame->width,
            tmp_frame->height, 1);
        buffer = (uint8_t*)av_malloc(size);
        if (!buffer) {
            fprintf(stderr, "Can not alloc buffer\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        ret = av_image_copy_to_buffer(buffer, size,
            (const uint8_t* const*)tmp_frame->data,
            (const int*)tmp_frame->linesize, (AVPixelFormat)tmp_frame->format,
            tmp_frame->width, tmp_frame->height, 1);
        if (ret < 0) {
            fprintf(stderr, "Can not copy image to buffer\n");
            goto fail;
        }

        if ((ret = fwrite(buffer, 1, size, output_file)) < 0) {
            fprintf(stderr, "Failed to dump raw data.\n");
            goto fail;
        }

    fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        av_freep(&buffer);
        if (ret < 0)
            return ret;
    }
}

int main(int argc, char* argv[])
{
    AVFormatContext* formatCtx = NULL;
    int video_stream, ret;
    AVStream* videoStream = NULL;
    AVCodecContext* codecCtx = NULL;
    const AVCodec* codec = NULL;
    AVPacket* packet = NULL;
    enum AVHWDeviceType hwType;     // cuda
    int i;

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <device type> <input file> <output file>\n", argv[0]);
        return -1;
    }

    hwType = av_hwdevice_find_type_by_name(argv[1]);
    if (hwType == AV_HWDEVICE_TYPE_NONE) {
        fprintf(stderr, "Device type %s is not supported.\n", argv[1]);
        fprintf(stderr, "Available device types:");
        while ((hwType = av_hwdevice_iterate_types(hwType)) != AV_HWDEVICE_TYPE_NONE)
            fprintf(stderr, " %s", av_hwdevice_get_type_name(hwType));
        fprintf(stderr, "\n");
        return -1;
    }

    packet = av_packet_alloc();
    if (!packet) {
        fprintf(stderr, "Failed to allocate AVPacket\n");
        return -1;
    }

    if (avformat_open_input(&formatCtx, argv[2], NULL, NULL) != 0) {
        fprintf(stderr, "Cannot open input file '%s'\n", argv[2]);
        return -1;
    }

    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        fprintf(stderr, "Cannot find input stream information.\n");
        return -1;
    }

    ret = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (ret < 0) {
        fprintf(stderr, "Cannot find a video stream in the input file\n");
        return -1;
    }
    video_stream = ret;

    // find out hardware output pixel format
    for (i = 0;; i++) {
        const AVCodecHWConfig* config = avcodec_get_hw_config(codec, i); 
        if (!config) {
            fprintf(stderr, "Decoder %s does not support device type %s.\n",
                codec->name, av_hwdevice_get_type_name(hwType));
            return -1;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == hwType) {
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }

    if (!(codecCtx = avcodec_alloc_context3(codec)))
        return AVERROR(ENOMEM);

    videoStream = formatCtx->streams[video_stream];
    if (avcodec_parameters_to_context(codecCtx, videoStream->codecpar) < 0)
        return -1;

    // setup callback function
    codecCtx->get_format = get_hw_format;

    // init codecCtx->hw_device_ctx
    if (hw_decoder_init(codecCtx, hwType) < 0)
        return -1;

    if ((ret = avcodec_open2(codecCtx, codec, NULL)) < 0) {
        fprintf(stderr, "Failed to open codec for stream #%u\n", video_stream);
        return -1;
    }

    output_file = fopen(argv[3], "w+b");

    while (ret >= 0) {
        if ((ret = av_read_frame(formatCtx, packet)) < 0)
            break;

        if (video_stream == packet->stream_index)
            ret = decode_write(codecCtx, packet);

        av_packet_unref(packet);
    }

    /* flush the decoder */
    ret = decode_write(codecCtx, nullptr);

    if (output_file)
        fclose(output_file);
    av_packet_free(&packet);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    av_buffer_unref(&hw_device_ctx);

    return 0;
}
