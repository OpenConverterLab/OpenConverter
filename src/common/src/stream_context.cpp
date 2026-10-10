/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
