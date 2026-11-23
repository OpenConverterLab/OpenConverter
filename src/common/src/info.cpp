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

#include "../include/info.h"
#include "../include/logger.h"

Info::Info() {
    avCtx = NULL;
    audioCodec = NULL;
    audioCtx = NULL;
    quickInfo = new QuickInfo();
    init();
}

void Info::init() {
    // Init QuickInfo
    quickInfo->videoIdx = -1;
    quickInfo->width = 0;
    quickInfo->height = 0;
    quickInfo->colorSpace = "";
    quickInfo->videoCodec = "";
    quickInfo->pixelFormat = "";
    quickInfo->videoBitRate = 0;
    quickInfo->frameRate = 0;

    quickInfo->audioIdx = -1;
    quickInfo->audioCodec = "";
    quickInfo->audioBitRate = 0;
    quickInfo->channels = 0;
    quickInfo->sampleFmt = "";
    quickInfo->sampleRate = 0;

    quickInfo->subIdx = -1;
}

void Info::print_error(const char *msg, int ret) {
    av_strerror(ret, errorMsg, sizeof(errorMsg));
    Logger::Log(std::string("Info: ") + (msg ? msg : "error") + ": " + errorMsg,
                true);
}

namespace {

bool HasCodecParameters(const AVFormatContext *ctx, int index) {
    return ctx && index >= 0 && index < static_cast<int>(ctx->nb_streams) &&
           ctx->streams[index] && ctx->streams[index]->codecpar;
}

void ReadVideoStream(QuickInfo *info, const AVStream *stream) {
    const AVCodecParameters *par = stream->codecpar;
    info->width = par->width;
    info->height = par->height;

    if (par->color_space != AVCOL_SPC_UNSPECIFIED) {
        const char *name = av_color_space_name(par->color_space);
        if (name)
            info->colorSpace = name;
    }
    if (par->codec_id != AV_CODEC_ID_NONE)
        info->videoCodec = avcodec_get_name(par->codec_id);

    const auto pixFmt = static_cast<AVPixelFormat>(par->format);
    if (pixFmt != AV_PIX_FMT_NONE) {
        const char *pixName = av_get_pix_fmt_name(pixFmt);
        if (pixName)
            info->pixelFormat = pixName;
    }
    info->videoBitRate = par->bit_rate;

    // Integer division turns 30000/1001 into 29 and faults when den is 0.
    AVRational rate = stream->avg_frame_rate;
    if (rate.num <= 0 || rate.den <= 0)
        rate = stream->r_frame_rate;
    if (rate.num > 0 && rate.den > 0) {
        info->frameRate = static_cast<double>(rate.num) /
                          static_cast<double>(rate.den);
    }
}

bool ReadAudioStream(QuickInfo *info, const AVCodecParameters *par,
                     AVCodecContext **audioCtx, const std::string &src) {
    *audioCtx = avcodec_alloc_context3(nullptr);
    if (!*audioCtx) {
        Logger::Log("Info: failed to allocate audio codec context.", true);
        return false;
    }
    if (avcodec_parameters_to_context(*audioCtx, par) < 0) {
        Logger::Log("Info: failed to copy audio codec parameters: " + src,
                    true);
        return false;
    }

    info->audioCodec = avcodec_get_name(par->codec_id);
    info->audioBitRate = par->bit_rate;
    info->channels = par->ch_layout.nb_channels;
    const char *sampleName = av_get_sample_fmt_name((*audioCtx)->sample_fmt);
    if (sampleName)
        info->sampleFmt = sampleName;
    info->sampleRate = par->sample_rate;
    return true;
}

} // namespace

QuickInfo *Info::get_quick_info() { return quickInfo; }

void Info::send_info(const std::string &src) {
    init();
    if (src.empty()) {
        Logger::Log("Info: empty media path.", true);
        return;
    }

    int ret = avformat_open_input(&avCtx, src.c_str(), nullptr, nullptr);
    if (ret < 0) {
        print_error("open failed", ret);
        Logger::Log("Info: failed to open input file: " + src, true);
        goto end;
    }

    ret = avformat_find_stream_info(avCtx, nullptr);
    if (ret < 0) {
        print_error("find stream info failed", ret);
        Logger::Log("Info: failed to read stream info: " + src, true);
    }

    {
        const int videoIdx =
            av_find_best_stream(avCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        const int audioIdx =
            av_find_best_stream(avCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        quickInfo->videoIdx = videoIdx >= 0 ? videoIdx : -1;
        quickInfo->audioIdx = audioIdx >= 0 ? audioIdx : -1;
    }

    if (HasCodecParameters(avCtx, quickInfo->videoIdx)) {
        ReadVideoStream(quickInfo, avCtx->streams[quickInfo->videoIdx]);
    } else if (quickInfo->videoIdx >= 0) {
        Logger::Log("Info: video stream has no codec parameters: " + src,
                    true);
        quickInfo->videoIdx = -1;
    }

    if (HasCodecParameters(avCtx, quickInfo->audioIdx)) {
        const AVCodecParameters *par =
            avCtx->streams[quickInfo->audioIdx]->codecpar;
        if (!ReadAudioStream(quickInfo, par, &audioCtx, src))
            goto end;
    } else if (quickInfo->audioIdx >= 0) {
        Logger::Log("Info: audio stream has no codec parameters: " + src,
                    true);
        quickInfo->audioIdx = -1;
    }

    if (quickInfo->videoIdx < 0 && quickInfo->audioIdx < 0)
        Logger::Log("Info: no audio or video stream in: " + src, true);

end:
    avformat_close_input(&avCtx);
    avcodec_free_context(&audioCtx);
}

Info::~Info() {
    delete quickInfo;
}
