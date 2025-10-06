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

#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <chrono>
#include <iostream>
#include <numeric>

#include "../../common/include/common.h"
#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../common/include/stream_context.h"

class Transcoder {
public:
    Transcoder(ProcessParameter *processParameter,
               EncodeParameter *encodeParameter)
        : processParameter(processParameter), encodeParameter(encodeParameter) {
        last_ui_update = std::chrono::system_clock::now();
    }

    virtual ~Transcoder() = default;

    virtual bool transcode(std::string input_path, std::string output_path) = 0;

    double compute_smooth_duration(double new_duration) {
        if (new_duration >= min_duration_threshold) {
            duration_history.push_back(new_duration);
            if (duration_history.size() > max_history_size) {
                duration_history.erase(duration_history.begin());
            }
        }
        return duration_history.empty()
                   ? 0.0
                   : std::accumulate(duration_history.begin(),
                                     duration_history.end(), 0.0) /
                         duration_history.size();
    }

    void send_process_parameter(int64_t frameNumber, int64_t frameTotalNumber) {
        processNumber = frameNumber * 100 / frameTotalNumber;

        static auto last_encoder_call_time = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - last_encoder_call_time)
                            .count();
        last_encoder_call_time = now;

        double smooth_duration = compute_smooth_duration(duration);
        if (frameNumber > 0 && frameTotalNumber > 0) {
            remainTime = smooth_duration * (100 - processNumber) / 1000;
        }

        // Only update UI if enough time has passed (100ms)
        auto time_since_last_ui_update =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_ui_update)
                .count();
        if (time_since_last_ui_update >= 100) {
            processParameter->set_Process_Number(processNumber);
            if (frameNumber > 0 && frameTotalNumber > 0) {
                processParameter->set_Time_Required(remainTime);
            }
            last_ui_update = now;
        }

        std::cout << "Process Number (percentage): " << processNumber << "%\t"
                  << "Current duration (milliseconds): " << duration << "\t"
                  << "Smoothed Duration: " << smooth_duration << " ms\t"
                  << "Estimated Rest Time (seconds): " << remainTime
                  << std::endl;
    }

    ProcessParameter *processParameter = NULL;
    EncodeParameter *encodeParameter = NULL;

    int64_t frameNumber = 0;
    int64_t frameTotalNumber = 0;
    int processNumber = 0;
    double remainTime = 0;

    std::chrono::system_clock::time_point
        last_ui_update; // Track last UI update time
    std::vector<double>
        duration_history; // Store recent durations for averaging

    static constexpr size_t max_history_size =
        20; // Limit for the number of durations tracked
    static constexpr double min_duration_threshold =
        10.0; // Ignore durations < 10 ms
};

#endif
