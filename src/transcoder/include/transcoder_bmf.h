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

#ifndef TRANSCODER_BMF_H
#define TRANSCODER_BMF_H

#include "transcoder.h"

#include "builder.hpp"
#include "nlohmann/json.hpp"

#include <regex>

class TranscoderBMF : public Transcoder {
public:
    TranscoderBMF(ProcessParameter *processParameter,
                  EncodeParameter *encodeParamter);

    bool prepare_info(std::string input_path, std::string output_path);

    bool transcode(std::string input_path, std::string output_path);

    bmf_sdk::CBytes decoder_callback(bmf_sdk::CBytes input);

    bmf_sdk::CBytes encoder_callback(bmf_sdk::CBytes input);

private:
    // encoder's parameters
    bool copyVideo;
    bool copyAudio;

    int width;
    int height;

    nlohmann::json decoder_para;
    nlohmann::json encoder_para;
};

#endif // TRANSCODER_BMF_H
