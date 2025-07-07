#include "../include/info.h"

Info::Info() {
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

QuickInfo *Info::get_Quick_Info() { return quickInfo; }

void Info::send_Info(char *src) {
    init();
    int ret = 0;
    av_log_set_level(AV_LOG_DEBUG);
    ret = avformat_open_input(&avCtx, src, NULL, NULL);
    if (ret < 0) {
        av_log(avCtx, AV_LOG_ERROR, "open failed");
        goto end;
    }
    ret = avformat_find_stream_info(avCtx, NULL);
    if (ret < 0) {
        CHECK_ERROR(ret);
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

#ifdef OC_FFMPEG_VERSION
    #if OC_FFMPEG_VERSION > 51
        quickInfo->colorSpace = av_color_space_name(
            avCtx->streams[quickInfo->videoIdx]->codecpar->color_space);
    #else
        quickInfo->colorSpace = av_get_colorspace_name(
            avCtx->streams[quickInfo->videoIdx]->codecpar->color_space);
    #endif
#endif
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
#ifdef OC_FFMPEG_VERSION
    #if OC_FFMPEG_VERSION < 60
    quickInfo->channels =
        avCtx->streams[quickInfo->audioIdx]->codecpar->channels;
    #else
    quickInfo->channels =
        avCtx->streams[quickInfo->audioIdx]->codecpar->ch_layout.nb_channels;
    #endif
#endif
    quickInfo->sampleFmt = av_get_sample_fmt_name(audioCtx->sample_fmt);
    quickInfo->sampleRate =
        avCtx->streams[quickInfo->audioIdx]->codecpar->sample_rate;

end:
    avformat_close_input(&avCtx);

    avcodec_free_context(&audioCtx);
}

Info::~Info() {}
