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
    // Probe the file at src. src is a UTF-8 filesystem path. FFmpeg's
    // file protocol accepts UTF-8 on every platform, including Windows,
    // where it converts the path to UTF-16. The string only needs to
    // stay alive for this call.
    void send_info(const std::string& src);
};

#endif // INFO_H
