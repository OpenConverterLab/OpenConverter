/*
 * Copyright 2024 Jack Lau
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
