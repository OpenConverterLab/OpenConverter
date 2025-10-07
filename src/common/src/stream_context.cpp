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
