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

#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <chrono>
#include <iostream>
#include <numeric>

#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../common/include/stream_context.h"

class Transcoder {
public:
    Transcoder(ProcessParameter *process_parameter,
               EncodeParameter *encode_parameter)
        : process_parameter(process_parameter), encode_parameter(encode_parameter) {
        first_frame_time = std::chrono::system_clock::time_point{};
        last_ui_update = std::chrono::system_clock::now();
    }

    virtual ~Transcoder() = default;

    virtual bool transcode(std::string input_path, std::string output_path) = 0;

    void send_process_parameter(int64_t frame_number, int64_t frame_total_number) {
        if (first_frame_time == std::chrono::system_clock::time_point{}) {
            first_frame_time = std::chrono::system_clock::now();
        }

        double fraction = 0.0;
        if (frame_total_number > 0) {
            fraction = static_cast<double>(frame_number) / static_cast<double>(frame_total_number);
            if (fraction > 1.0) fraction = 1.0; // clamp just in case
            if (fraction < 0.0) fraction = 0.0;
        }
        process_number = static_cast<int>(fraction * 100.0);

        auto now = std::chrono::system_clock::now();

        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - first_frame_time)
                            .count();

        if (frame_number > 0 && frame_total_number > 0) {
            double elapsed_ms_d = static_cast<double>(elapsed_ms);
            remain_seconds = (elapsed_ms / fraction - elapsed_ms_d) / 1000.0;
        }

        // Only update UI if enough time has passed (100ms)
        auto time_since_last_ui_update =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_ui_update)
                .count();
        if (time_since_last_ui_update >= 100) {
            process_parameter->set_process_number(process_number);
            if (frame_number > 0 && frame_total_number > 0) {
                process_parameter->set_time_required(remain_seconds);
            }
            last_ui_update = now;
        }

        std::cout << "Process Number (percentage): " << process_number << "%\t"
                  << "Elapsed Time (milliseconds): " << elapsed_ms << "\t"
                  << "Estimated Rest Time (seconds): " << remain_seconds
                  << std::endl;
    }

    ProcessParameter *process_parameter = NULL;
    EncodeParameter *encode_parameter = NULL;

    int64_t frame_number = 0;
    int64_t frame_total_number = 0;
    int process_number = 0;
    double remain_seconds = 0;

    std::chrono::system_clock::time_point first_frame_time;
    std::chrono::system_clock::time_point
        last_ui_update; // Track last UI update time

};

#endif
