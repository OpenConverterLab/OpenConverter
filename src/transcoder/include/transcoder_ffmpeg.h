/*
 * Copyright 2024 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

#ifndef TRANSCODERFFMPEG_H
#define TRANSCODERFFMPEG_H

#include "transcoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
};

#define ENCODE_BIT_RATE 5000000

typedef struct FilteringContext {
    AVFilterContext *buffersrc_ctx;
    AVFilterContext *buffersink_ctx;
    AVFilterGraph *filter_graph;
} FilteringContext;

class TranscoderFFmpeg : public Transcoder {
public:
    TranscoderFFmpeg(ProcessParameter *processParameter,
                     EncodeParameter *encodeParameter);
    ~TranscoderFFmpeg();

    bool transcode(std::string input_path, std::string output_path);

    int open_media();

    int init_filter(AVCodecContext *dec_ctx, FilteringContext *filter_ctx, const char *filters_descr);

    int init_filters_wrapper();

    int encode_video(AVStream *inStream, AVFrame *frame);

    int encode_write_video(AVFrame *frame);

    int transcode_video();

    int encode_audio(AVStream *inStream, AVFrame *frame);

    int encode_write_audio(AVFrame *frame);

    int transcode_audio();

    int prepare_decoder();

    int prepare_encoder_video();

    int prepare_encoder_audio();

    int prepare_copy(AVFormatContext *avCtx, AVStream **stream,
                     AVCodecParameters *codecParam);

    int remux(AVPacket *pkt, AVFormatContext *avCtx, AVStream *inStream,
              AVStream *outStream);

private:
    char errorMsg[128];
    // encoder's parameters
    bool copyVideo;
    bool copyAudio;

    // Decoder and encoder contexts
    StreamContext *decoder;
    StreamContext *encoder;

    FilteringContext *filters_ctx;

    // Progress tracking
    int64_t total_duration;   // Total duration in microseconds
    int64_t current_duration; // Current processed duration in microseconds

    // Helper function to update progress
    void update_progress(int64_t current_pts, AVRational time_base);
    void print_error(const char *msg, int ret);
};

#endif // TRANSCODERFFMPEG_H
