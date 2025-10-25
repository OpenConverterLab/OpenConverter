/*
 * Copyright 2025 Jack Lau
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

#include "../include/stream_context.h"

StreamContext::StreamContext() {
    fmtCtx = NULL;
    filename = NULL;

    videoIdx = OC_INVALID_STREAM_IDX;
    videoStream = NULL;
    videoCodec = NULL;
    videoCodecCtx = NULL;

    audioIdx = OC_INVALID_STREAM_IDX;
    audioStream = NULL;
    audioCodec = NULL;
    audioCodecCtx = NULL;

    pkt = NULL;
    frame = NULL;
}

StreamContext::~StreamContext() {
    if (videoCodecCtx) {
        avcodec_free_context(&videoCodecCtx);
        videoCodecCtx = NULL;
    }
    if (audioCodecCtx) {
        avcodec_free_context(&audioCodecCtx);
        audioCodecCtx = NULL;
    }
    if (pkt) {
        av_packet_free(&pkt);
        pkt = NULL;
    }
    if (frame) {
        av_frame_free(&frame);
        frame = NULL;
    }
}
