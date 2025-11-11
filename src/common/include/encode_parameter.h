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

    std::string pixelFormat;
    uint16_t width;
    uint16_t height;

    std::string audioCodec;
    int64_t audioBitRate;

    int qscale;

    std::string preset;

    double startTime;  // in seconds
    double endTime;    // in seconds

public:
    EncodeParameter();
    ~EncodeParameter();

    bool get_available();

    void set_video_codec_name(std::string vc);

    void set_qscale(int q);

    void set_pixel_format(std::string p);

    void set_width(uint16_t w);

    void set_height(uint16_t h);

    void set_audio_codec_name(std::string ac);

    void set_video_bit_rate(int64_t vbr);

    void set_audio_bit_rate(int64_t abr);

    void set_preset(std::string p);

    void set_start_time(double t);

    void set_end_time(double t);

    std::string get_video_codec_name();

    int get_qscale();

    std::string get_pixel_format();

    uint16_t get_width();

    uint16_t get_height();

    std::string get_audio_codec_name();

    int64_t get_video_bit_rate();

    int64_t get_audio_bit_rate();

    std::string get_preset();

    double get_start_time();

    double get_end_time();
};

#endif // ENCODEPARAMETER_H
