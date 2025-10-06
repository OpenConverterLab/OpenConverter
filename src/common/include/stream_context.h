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

#ifndef STREAMCONTEXT_H
#define STREAMCONTEXT_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
};

#define OC_INVALID_STREAM_IDX -1

class StreamContext {

public:
    StreamContext();
    ~StreamContext();

    AVFormatContext *fmtCtx;
    const char *filename;

    int videoIdx;
    AVStream *videoStream;
    const AVCodec *videoCodec;
    AVCodecContext *videoCodecCtx;

    int audioIdx;
    AVStream *audioStream;
    const AVCodec *audioCodec;
    AVCodecContext *audioCodecCtx;

    AVPacket *pkt;
    AVFrame *frame;
};

#endif // STREAMCONTEXT_H
