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

StreamContext::~StreamContext() {}
