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

#ifndef INFO_H
#define INFO_H

#include <map>
#include <stdint.h>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
};

// store some info of video and audio
typedef struct QuickInfo {
    // video
    int videoIdx;
    int width;
    int height;

    std::string colorSpace;
    std::string videoCodec;
    std::string pixelFormat;

    int64_t videoBitRate;
    double frameRate;
    // audio
    int audioIdx;
    std::string audioCodec;
    int64_t audioBitRate;
    int channels;
    std::string sampleFmt;
    int sampleRate;

    // subtitle
    int subIdx;
    std::string subCodec;
    std::string subFmt;
    std::string displayTime;
    int position;
    // QSize subSize;
    std::string subColor;
} QuickInfo;

// deal with info of video and audio and stored as QuickInfo type
class Info {

public:
    Info();
    ~Info();

private:
    void print_error(const char *msg, int ret);

    AVFormatContext *avCtx;

    const AVCodec *audioCodec;

    AVCodecContext *audioCtx;

    QuickInfo *quickInfo;

    char errorMsg[128];
public:
    // init quick info
    void init();
    // get qucik info reference
    QuickInfo *get_quick_info();
    // send the info to front-end
    void send_info(char *src);
};

#endif // INFO_H
