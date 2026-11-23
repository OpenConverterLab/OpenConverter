/*
 * Copyright 2025 Jack Lau
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

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#include "../include/transcoder_fftool.h"
#include "../../common/include/logger.h"

TranscoderFFTool::TranscoderFFTool(ProcessParameter *process_parameter,
                                   EncodeParameter *encode_parameter)
    : Transcoder(process_parameter, encode_parameter), copy_video(false),
      copy_audio(false), frame_total_number(0) {}

TranscoderFFTool::~TranscoderFFTool() {
    // Destructor implementation
}

// Helper function to escape file paths for Windows
std::string escape_windows_path(const std::string &path) {
    std::string escaped = path;
    size_t pos = 0;
    while ((pos = escaped.find("\\", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\\\");
        pos += 2;
    }
    return escaped;
}

bool TranscoderFFTool::prepared_opt() {
    // The old order dereferenced encode_parameter before the null check.
    if (!encode_parameter)
        return false;

    video_codec = encode_parameter->get_video_codec_name();
    video_bit_rate = encode_parameter->get_video_bit_rate();
    audio_codec = encode_parameter->get_audio_codec_name();
    audio_bit_rate = encode_parameter->get_audio_bit_rate();
    width = encode_parameter->get_width();
    height = encode_parameter->get_height();
    qscale = encode_parameter->get_qscale();
    pixel_format = encode_parameter->get_pixel_format();
    start_time = encode_parameter->get_start_time();
    end_time = encode_parameter->get_end_time();

    copy_video = video_codec == "copy";
    copy_audio = audio_codec == "copy";
    return true;
}

bool TranscoderFFTool::transcode(std::string input_path,
                                 std::string output_path) {
    if (!prepared_opt()) {
        Logger::Log("FFTool: Failed to prepare options for transcoding.", true);
        return false;
    }

// Convert paths for Windows (escape backslashes)
#ifdef _WIN32
    input_path = escape_windows_path(input_path);
    output_path = escape_windows_path(output_path);
#endif

    // Build the FFmpeg command
    std::stringstream cmd;

// Check if FFMPEG_PATH is defined (ensure it's set by CMake)
#ifdef FFTOOL_PATH
    cmd << "\"" << FFTOOL_PATH << "\"";

    // Add start time seeking if specified (before -i for faster seeking)
    if (start_time > 0) {
        cmd << " -ss " << start_time;
    }

    cmd << " -i \"" << input_path << "\"";
#else
    Logger::Log("FFTool: ffmpeg path is not defined. CMake must set FFTOOL_PATH.", true);
    return false;
#endif

    // Add the -y flag to overwrite output file without prompting
    cmd << " -y";

    // Add end time or duration if specified
    if (end_time > 0) {
        if (start_time > 0) {
            // If both start and end are specified, use duration
            double cut_duration = end_time - start_time;
            cmd << " -t " << cut_duration;
        } else {
            // If only end time is specified, use -to
            cmd << " -to " << end_time;
        }
    }

    // Video codec options
    if (copy_video) {
        cmd << " -c:v copy"; // Copy video stream without re-encoding
    } else {
        if (!video_codec.empty()) {
            cmd << " -c:v " << video_codec; // Use specified video codec
        } else {
            Logger::Log("FFTool: Video codec is not specified!", true);
            return false;
        }
        if (video_bit_rate > 0) {
            cmd << " -b:v " << video_bit_rate; // Set video bitrate if specified
        }

        // Add qscale (quality) if specified (qscale >= 0)
        if (qscale >= 0) {
            cmd << " -qscale:v " << qscale;
        }

        // Add pixel format if specified
        if (!pixel_format.empty()) {
            cmd << " -pix_fmt " << pixel_format;
        }

        // Add scale filter if width or height is specified
        if (width > 0 || height > 0) {
            std::string scale_filter = "scale=";
            if (width > 0) {
                scale_filter += std::to_string(width);
            } else {
                scale_filter += "-1";  // Keep aspect ratio
            }
            scale_filter += ":";
            if (height > 0) {
                scale_filter += std::to_string(height);
            } else {
                scale_filter += "-1";  // Keep aspect ratio
            }
            cmd << " -vf " << scale_filter;
        }
    }

    // Audio codec options
    if (copy_audio) {
        cmd << " -c:a copy"; // Copy audio stream without re-encoding
    } else {
        if (!audio_codec.empty()) {
            cmd << " -c:a " << audio_codec; // Use specified audio codec
        } else {
            Logger::Log("FFTool: Audio codec is not specified!", true);
            return false;
        }
        if (audio_bit_rate > 0) {
            cmd << " -b:a " << audio_bit_rate; // Set audio bitrate if specified
        }
    }

    // Output file path
    cmd << " \"" << output_path << "\"";

    // The previous Windows path wrapped this string in an extra quote and
    // logged a different command than the one Unix actually ran.
    const std::string command = cmd.str();
    Logger::Log("FFTool executing: " + command);

    int status = std::system(command.c_str());
    if (status == -1) {
        Logger::Log("FFTool: failed to start ffmpeg.", true);
        return false;
    }

#ifndef _WIN32
    // system() returns a wait status, not the process exit code.
    if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        Logger::Log("FFTool: ffmpeg was killed by signal " +
                        std::to_string(WTERMSIG(status)),
                    true);
        return false;
    } else {
        Logger::Log("FFTool: ffmpeg returned unexpected status " +
                        std::to_string(status),
                    true);
        return false;
    }
#endif

    if (status != 0) {
        Logger::Log("FFTool: ffmpeg failed with exit code " +
                        std::to_string(status),
                    true);
        return false;
    }

    Logger::Log("FFTool: transcoding completed successfully.");
    return true;
}
