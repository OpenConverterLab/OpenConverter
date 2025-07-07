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

#ifndef TRANSCODERFFMPEG_H
#define TRANSCODERFFMPEG_H

#include "transcoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
};

#define ENCODE_BIT_RATE 5000000

class TranscoderFFmpeg : public Transcoder {
  public:
    TranscoderFFmpeg(ProcessParameter *processParameter,
                     EncodeParameter *encodeParameter);
    ~TranscoderFFmpeg();

    bool transcode(std::string input_path, std::string output_path);

    bool open_Media(StreamContext *decoder, StreamContext *encoder);

    bool copyFrame(AVFrame *oldFrame, AVFrame *newFrame);

    bool encode_Video(AVStream *inStream, StreamContext *encoder,
                      AVFrame *inputFrame);

    bool transcode_Video(StreamContext *decoder, StreamContext *encoder);

    bool encode_Audio(AVStream *inStream, StreamContext *encoder,
                      AVFrame *inputFrame);

    bool transcode_Audio(StreamContext *decoder, StreamContext *encoder);

    bool prepare_Decoder(StreamContext *decoder);

    bool prepare_Encoder_Video(StreamContext *decoder, StreamContext *encoder);

    bool prepare_Encoder_Audio(StreamContext *decoder, StreamContext *encoder);

    bool prepare_Copy(AVFormatContext *avCtx, AVStream **stream,
                      AVCodecParameters *codecParam);

    bool remux(AVPacket *pkt, AVFormatContext *avCtx, AVStream *inStream,
               AVStream *outStream);

  private:
    // encoder's parameters
    bool copyVideo;
    bool copyAudio;
    
    // Progress tracking
    int64_t total_duration;  // Total duration in microseconds
    int64_t current_duration;  // Current processed duration in microseconds
    
    // Helper function to update progress
    void update_progress(int64_t current_pts, AVRational time_base);
};

#endif // TRANSCODERFFMPEG_H
