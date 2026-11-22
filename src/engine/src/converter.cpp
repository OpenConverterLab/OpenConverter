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

#include "../../common/include/logger.h"

#if defined(ENABLE_BMF)
    #include "../../transcoder/include/transcoder_bmf.h"
#endif
#if defined(ENABLE_FFMPEG)
    #include "../../transcoder/include/transcoder_ffmpeg.h"
#endif
#if defined(ENABLE_FFTOOL)
    #include "../../transcoder/include/transcoder_fftool.h"
#endif

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

    this->encodeParameter = encodeParamter;
}

bool Converter::set_transcoder(std::string transcoderName) {
    delete transcoder;
    transcoder = nullptr;

    if (transcoderName == "FFMPEG") {
#if defined(ENABLE_FFMPEG)
        transcoder = new TranscoderFFmpeg(this->processParameter,
                                          this->encodeParameter);
#endif
    } else if (transcoderName == "BMF") {
#if defined(ENABLE_BMF)
        transcoder = new TranscoderBMF(this->processParameter,
                                       this->encodeParameter);
#endif
    } else if (transcoderName == "FFTOOL") {
#if defined(ENABLE_FFTOOL)
        transcoder = new TranscoderFFTool(this->processParameter,
                                          this->encodeParameter);
#endif
    } else {
        Logger::Log("Unknown transcoder name: " + transcoderName, true);
        return false;
    }

    // The name can be recognized while that backend was compiled out.
    // Returning success with a null transcoder used to crash the next call.
    if (!transcoder) {
        Logger::Log("Failed to create transcoder: " + transcoderName, true);
        return false;
    }

    Logger::Log("Using transcoder: " + transcoderName);
    return true;
}

bool Converter::convert_format(const std::string &src, const std::string &dst) {
    if (!transcoder) {
        Logger::Log("Conversion requested with no transcoder.", true);
        return false;
    }
    if (src.empty() || dst.empty()) {
        Logger::Log("Conversion requested with an empty path.", true);
        return false;
    }
    return transcoder->transcode(src, dst);
}

Converter::~Converter() {
    if (transcoder) {
        delete transcoder;
    }
}
