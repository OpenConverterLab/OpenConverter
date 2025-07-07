#include "../include/stream_context.h"

StreamContext::StreamContext() {
    fmtCtx = NULL;
    filename = NULL;

    videoIdx = 0;
    videoStream = NULL;
    videoCodec = NULL;
    videoCodecCtx = NULL;

    audioIdx = 0;
    audioStream = NULL;
    audioCodec = NULL;
    audioCodecCtx = NULL;

    pkt = NULL;
    frame = NULL;
}

StreamContext::~StreamContext() {}
