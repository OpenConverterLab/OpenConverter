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

#ifndef CONVERTER_H
#define CONVERTER_H

#include "../../common/include/encode_parameter.h"
#include "../../transcoder/include/transcoder.h"
#include <functional>
#include <string>

class Converter {
public:
    Converter();
    Converter(ProcessParameter *processParamter,
              EncodeParameter *encodeParamter);
    ~Converter();

    bool set_Transcoder(std::string transcoderName);
    bool convert_Format(const std::string &src, const std::string &dst);

private:
    Transcoder *transcoder = NULL;
    bool copyVideo;
    bool copyAudio;

public:
    ProcessParameter *processParameter = NULL;
    EncodeParameter *encodeParameter = NULL;
};

#endif // CONVERTER_H
