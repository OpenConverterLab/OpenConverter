/*
 * Copyright 2025 Jack Lau
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

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include "../include/transcoder_fftool.h"

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

    if (encode_parameter->get_video_codec_name() == "copy") {
        copy_video = true;
    } else {
        copy_video = false;
    }

    if (encode_parameter->get_audio_codec_name() == "copy") {
        copy_audio = true;
    } else {
        copy_audio = false;
    }

    if (encode_parameter) {
        video_codec = encode_parameter->get_video_codec_name();
        video_bit_rate = encode_parameter->get_video_bit_rate();
        audio_codec = encode_parameter->get_audio_codec_name();
        audio_bit_rate = encode_parameter->get_audio_bit_rate();

        // Get additional video parameters
        width = encode_parameter->get_width();
        height = encode_parameter->get_height();
        qscale = encode_parameter->get_qscale();
        pixel_format = encode_parameter->get_pixel_format();

        // Get time range parameters
        start_time = encode_parameter->get_start_time();
        end_time = encode_parameter->get_end_time();
    }

    return true;
}

bool TranscoderFFTool::transcode(std::string input_path,
                                 std::string output_path) {
    if (!prepared_opt()) {
        std::cerr << "Failed to prepare options for transcoding." << std::endl;
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
    std::cerr << "FFmpeg path is not defined! Ensure CMake sets FFMPEG_PATH."
              << std::endl;
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
            std::cerr << "Video codec is not specified!" << std::endl;
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
            std::cerr << "Audio codec is not specified!" << std::endl;
            return false;
        }
        if (audio_bit_rate > 0) {
            cmd << " -b:a " << audio_bit_rate; // Set audio bitrate if specified
        }
    }

    // Output file path
    cmd << " \"" << output_path << "\"";

    std::string raw_cmd = cmd.str();
    std::string final_cmd = "\"" + raw_cmd;

    // Execute the command
    std::cout << "Executing: " << final_cmd << std::endl;

    int ret = 0;

#ifdef _WIN32
    // Windows-specific command execution (use cmd /c for shell commands)
    std::string full_cmd = "cmd /c " + final_cmd;
    ret = system(full_cmd.c_str());
#else
    // Unix-like systems (Linux/macOS) can directly use system()
    ret = system(cmd.str().c_str());
#endif

    if (ret != 0) {
        std::cerr << "FFmpeg transcoding failed with exit code: " << ret
                  << std::endl;
        return false;
    }

    std::cout << "Transcoding completed successfully." << std::endl;
    return true;
}
