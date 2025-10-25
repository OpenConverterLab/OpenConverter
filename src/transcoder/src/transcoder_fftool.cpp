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

TranscoderFFTool::TranscoderFFTool(ProcessParameter *processParameter,
                                   EncodeParameter *encodeParameter)
    : Transcoder(processParameter, encodeParameter), copyVideo(false),
      copyAudio(false), frameTotalNumber(0) {}

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

    if (encodeParameter->get_video_codec_name() == "") {
        copyVideo = true;
    } else {
        copyVideo = false;
    }

    if (encodeParameter->get_audio_codec_name() == "") {
        copyAudio = true;
    } else {
        copyAudio = false;
    }

    if (encodeParameter) {
        videoCodec = encodeParameter->get_video_codec_name();
        videoBitRate = encodeParameter->get_video_bit_rate();
        audioCodec = encodeParameter->get_audio_codec_name();
        audioBitRate = encodeParameter->get_audio_bit_rate();
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
    cmd << "\"" << FFTOOL_PATH << "\" -i \"" << input_path << "\"";
#else
    std::cerr << "FFmpeg path is not defined! Ensure CMake sets FFMPEG_PATH."
              << std::endl;
    return false;
#endif

    // Add the -y flag to overwrite output file without prompting
    cmd << " -y";

    // Video codec options
    if (copyVideo) {
        cmd << " -c:v copy"; // Copy video stream without re-encoding
    } else {
        if (!videoCodec.empty()) {
            cmd << " -c:v " << videoCodec; // Use specified video codec
        } else {
            std::cerr << "Video codec is not specified!" << std::endl;
            return false;
        }
        if (videoBitRate > 0) {
            cmd << " -b:v " << videoBitRate; // Set video bitrate if specified
        }
    }

    // Audio codec options
    if (copyAudio) {
        cmd << " -c:a copy"; // Copy audio stream without re-encoding
    } else {
        if (!audioCodec.empty()) {
            cmd << " -c:a " << audioCodec; // Use specified audio codec
        } else {
            std::cerr << "Audio codec is not specified!" << std::endl;
            return false;
        }
        if (audioBitRate > 0) {
            cmd << " -b:a " << audioBitRate; // Set audio bitrate if specified
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
    std::string fullCmd = "cmd /c " + final_cmd;
    ret = system(fullCmd.c_str());
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
