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

#ifndef ENCODEPARAMETER_H
#define ENCODEPARAMETER_H

#include <cstdint>
#include <string>

class EncodeParameter {
private:
    bool available;

    std::string videoCodec;
    int64_t videoBitRate;

    std::string audioCodec;
    int64_t audioBitRate;

    int qscale;

public:
    EncodeParameter();
    ~EncodeParameter();

    bool get_available();

    void set_video_codec_name(std::string vc);

    void set_qscale(int q);

    void set_audio_codec_name(std::string ac);

    void set_video_bit_rate(int64_t vbr);

    void set_audio_bit_rate(int64_t abr);

    std::string get_video_codec_name();

    int get_qscale();

    std::string get_audio_codec_name();

    int64_t get_video_bit_rate();

    int64_t get_audio_bit_rate();
};

#endif // ENCODEPARAMETER_H
