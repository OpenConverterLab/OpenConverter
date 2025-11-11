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

#ifndef TRANSCODERFFTOOL_H
#define TRANSCODERFFTOOL_H

#include "transcoder.h"

class TranscoderFFTool : public Transcoder {
public:
    TranscoderFFTool(ProcessParameter *process_parameter,
                     EncodeParameter *encode_parameter);
    ~TranscoderFFTool();

    bool prepared_opt();

    bool transcode(std::string input_path, std::string output_path);

private:
    // encoder's parameters
    bool copy_video;
    bool copy_audio;

    std::string video_codec;
    int64_t video_bit_rate;
    std::string audio_codec;
    int64_t audio_bit_rate;

    // Additional video parameters
    uint16_t width;
    uint16_t height;
    int qscale;
    std::string pixel_format;

    // Time range parameters
    double start_time;  // in seconds
    double end_time;    // in seconds

    static int frame_number;

    int64_t frame_total_number;
};

#endif // TRANSCODERFFTOOL_H
