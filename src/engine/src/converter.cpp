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

    return transcoder->transcode(src, dst);
}

Converter::~Converter() {
    if (transcoder) {
        delete transcoder;
    }
}
