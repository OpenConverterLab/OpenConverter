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

#ifndef TRANSCODER_BMF_H
#define TRANSCODER_BMF_H

#include "transcoder.h"

#include "builder.hpp"
#include "nlohmann/json.hpp"

#include <regex>

class TranscoderBMF : public Transcoder {
public:
    TranscoderBMF(ProcessParameter *process_parameter,
                  EncodeParameter *encode_parameter);

    bool prepare_info(std::string input_path, std::string output_path);

    bool transcode(std::string input_path, std::string output_path);

    bmf_sdk::CBytes decoder_callback(bmf_sdk::CBytes input);

    bmf_sdk::CBytes encoder_callback(bmf_sdk::CBytes input);

private:
    // encoder's parameters
    bool copy_video;
    bool copy_audio;

    int width;
    int height;

    nlohmann::json decoder_para;
    nlohmann::json encoder_para;

    // Helper function to set up Python environment (PYTHONPATH)
    // Returns true if setup succeeded, false if App Python is not installed
    bool setup_python_environment();

    // Helper function to get the Python module path
    std::string get_python_module_path();
};

#endif // TRANSCODER_BMF_H
