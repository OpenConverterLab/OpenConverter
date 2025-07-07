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

#ifndef ENCODEPARAMETER_H
#define ENCODEPARAMETER_H

#include <cstdint>
#include <string>

class EncodeParameter {
  private:
    bool available;

    std::string videoCodec;
    int64_t videoBitRate;

    std::string audioCodec;
    int64_t audioBitRate;

  public:
    EncodeParameter();
    ~EncodeParameter();

    bool get_Available();

    void set_Video_Codec_Name(std::string vc);

    void set_Audio_Codec_Name(std::string ac);

    void set_Video_Bit_Rate(int64_t vbr);

    void set_Audio_Bit_Rate(int64_t abr);

    std::string get_Video_Codec_Name();

    std::string get_Audio_Codec_Name();

    int64_t get_Video_Bit_Rate();

    int64_t get_Audio_Bit_Rate();
};

#endif // ENCODEPARAMETER_H
