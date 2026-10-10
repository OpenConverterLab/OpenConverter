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
