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

#include "../include/info.h"

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
    quickInfo->videoBitRate = 0;
    quickInfo->frameRate = 0;

    quickInfo->audioIdx = -1;
    quickInfo->audioCodec = "";
    quickInfo->audioBitRate = 0;
    quickInfo->channels = 0;
    quickInfo->sampleFmt = "";
    quickInfo->sampleRate = 0;

    quickInfo->subIdx = 0;
}

void Info::print_error(const char *msg, int ret) {
    av_strerror(ret, errorMsg, sizeof(errorMsg));
    av_log(NULL, AV_LOG_ERROR, " %s: %s \n", msg, errorMsg);
}


QuickInfo *Info::get_quick_info() { return quickInfo; }

void Info::send_info(char *src) {
    init();
    int ret = 0;
    av_log_set_level(AV_LOG_DEBUG);
    ret = avformat_open_input(&avCtx, src, NULL, NULL);
    if (ret < 0) {
        print_error("open failed", ret);
        goto end;
    }
    ret = avformat_find_stream_info(avCtx, NULL);
    if (ret < 0) {
        print_error("find stream info failed", ret);
    }
    // find the video and audio stream from container
    quickInfo->videoIdx =
        av_find_best_stream(avCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    quickInfo->audioIdx =
        av_find_best_stream(avCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    if (quickInfo->videoIdx >= 0) {
        quickInfo->height =
            avCtx->streams[quickInfo->videoIdx]->codecpar->height;
        quickInfo->width = avCtx->streams[quickInfo->videoIdx]->codecpar->width;

        if (avCtx->streams[quickInfo->videoIdx]->codecpar->color_space != AVCOL_SPC_UNSPECIFIED) {
            quickInfo->colorSpace = av_color_space_name(
                avCtx->streams[quickInfo->videoIdx]->codecpar->color_space);
        }
        if (avCtx->streams[quickInfo->videoIdx]->codecpar->codec_id != AV_CODEC_ID_NONE)
            quickInfo->videoCodec = avcodec_get_name(
                avCtx->streams[quickInfo->videoIdx]->codecpar->codec_id);
        quickInfo->videoBitRate =
            avCtx->streams[quickInfo->videoIdx]->codecpar->bit_rate;
        quickInfo->frameRate =
            avCtx->streams[quickInfo->videoIdx]->r_frame_rate.num /
            avCtx->streams[quickInfo->videoIdx]->r_frame_rate.den;

    } else {
        av_log(avCtx, AV_LOG_ERROR, "There is no video stream!\n");
        // goto end;
    }

    if (quickInfo->audioIdx < 0) {
        av_log(avCtx, AV_LOG_ERROR, "There is no audio stream!\n");
        goto end;
    }

    audioCtx = avcodec_alloc_context3(NULL);
    if (!audioCtx) {
        av_log(audioCtx, AV_LOG_ERROR,
               "Could not allocate audio codec context\n");
        goto end;
    }
    // Open the audio codec
    if (avcodec_parameters_to_context(
            audioCtx, avCtx->streams[quickInfo->audioIdx]->codecpar) < 0) {
        av_log(avCtx, AV_LOG_ERROR, "Failed to initialize codec context\n");
        goto end;
    }

    quickInfo->audioCodec = avcodec_get_name(
        avCtx->streams[quickInfo->audioIdx]->codecpar->codec_id);
    quickInfo->audioBitRate =
        avCtx->streams[quickInfo->audioIdx]->codecpar->bit_rate;
    quickInfo->channels =
        avCtx->streams[quickInfo->audioIdx]->codecpar->ch_layout.nb_channels;
    quickInfo->sampleFmt = av_get_sample_fmt_name(audioCtx->sample_fmt);
    quickInfo->sampleRate =
        avCtx->streams[quickInfo->audioIdx]->codecpar->sample_rate;

end:
    avformat_close_input(&avCtx);

    avcodec_free_context(&audioCtx);
}

Info::~Info() {
    delete quickInfo;
}
