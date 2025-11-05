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

#include "../include/converter.h"

#if defined(ENABLE_BMF)
    #include "../../transcoder/include/transcoder_bmf.h"
#endif
#if defined(ENABLE_FFMPEG)
    #include "../../transcoder/include/transcoder_ffmpeg.h"
#endif
#if defined(ENABLE_FFTOOL)
    #include "../../transcoder/include/transcoder_fftool.h"
#endif

// Required for debug logging
#include <iostream>

Converter::Converter() {}
/* Receive pointers from widget */
Converter::Converter(ProcessParameter *processParamter,
                     EncodeParameter *encodeParamter)
    : processParameter(processParamter), encodeParameter(encodeParamter) {
// #if defined(USE_BMF)
//     transcoder = new TranscoderBMF(this->processParameter,
//     this->encodeParameter);
// #elif defined(USE_FFMPEG)
//     transcoder = new TranscoderFFmpeg(this->processParameter,
//     this->encodeParameter);
// #elif defined(USE_FFTOOL)
//     transcoder = new TranscoderFFTool(this->processParameter,
//     this->encodeParameter);
// #endif

// Default transcoder
#if defined(ENABLE_FFMPEG)
    transcoder =
        new TranscoderFFmpeg(this->processParameter, this->encodeParameter);
#endif

    // Removed redundant assignment as per feedback (encodeParameter is already initialized)
    // this->encodeParameter = encodeParamter;
}

bool Converter::set_transcoder(std::string transcoderName) {
    if (transcoder) {
        delete transcoder;
        transcoder = NULL;
    }

    if (transcoder == NULL) {
        if (transcoderName == "FFMPEG") {
#if defined(ENABLE_FFMPEG)
            transcoder = new TranscoderFFmpeg(this->processParameter,
                                              this->encodeParameter);
#endif
            std::cout << "Set FFmpeg Transcoder!" << std::endl;
        } else if (transcoderName == "BMF") {
#if defined(ENABLE_BMF)
            transcoder = new TranscoderBMF(this->processParameter,
                                           this->encodeParameter);
            std::cout << "Set BMF Transcoder!" << std::endl;
#endif
        } else if (transcoderName == "FFTOOL") {
#if defined(ENABLE_FFTOOL)
            transcoder = new TranscoderFFTool(this->processParameter,
                                              this->encodeParameter);
            std::cout << "Set FFTool Transcoder!" << std::endl;
#endif
        } else {
            std::cout << "Wrong Transcoder Name!" << std::endl;
            return false;
        }
    } else {
        std::cout << "Init transcoder failed!" << std::endl;
        return false;
    }
    return true;
}

bool Converter::convert_format(const std::string &src, const std::string &dst) {
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

    // Handle video cutting parameters if enabled
    if (processParameter && processParameter->get_enable_cut()) {
        double start_time = processParameter->get_cut_start_time();
        double end_time = processParameter->get_cut_end_time();

        // Validate cutting parameters
        if (start_time < 0) {
            std::cerr << "Converter Error: Cut start time (" << start_time << ") cannot be negative. Aborting conversion." << std::endl;
            return false;
        }
        if (end_time <= start_time) {
            std::cerr << "Converter Error: Cut end time (" << end_time << ") must be greater than start time (" << start_time << "). Aborting conversion." << std::endl;
            return false;
        }

        double duration = end_time - start_time;

        // Debug logs for cutting parameters
        std::cout << "Converter Debug: Video cutting enabled." << std::endl;
        std::cout << "Converter Debug: Cut Start Time: " << start_time << " seconds." << std::endl;
        std::cout << "Converter Debug: Cut End Time: " << end_time << " seconds." << std::endl;
        std::cout << "Converter Debug: Calculated Clip Duration: " << duration << " seconds." << std::endl;

        // Store the calculated duration back into processParameter for the transcoder to use.
        // This assumes that ProcessParameter has a setter like set_cut_duration(double).
        // This ensures the transcoder has a pre-calculated duration available, addressing the feedback.
        processParameter->set_cut_duration(duration);
    }

    return transcoder->transcode(src, dst);
}

Converter::~Converter() {
    if (transcoder) {
        delete transcoder;
    }
}