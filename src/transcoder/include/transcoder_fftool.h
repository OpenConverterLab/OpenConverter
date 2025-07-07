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

#ifndef TRANSCODERFFTOOL_H
#define TRANSCODERFFTOOL_H

#include "transcoder.h"

class TranscoderFFTool : public Transcoder {
  public:
    TranscoderFFTool(ProcessParameter *processParameter,
                     EncodeParameter *encodeParameter);
    ~TranscoderFFTool();

    bool prepared_opt();

    bool transcode(std::string input_path, std::string output_path);

  private:
    // encoder's parameters
    bool copyVideo;
    bool copyAudio;

    std::string videoCodec;
    int64_t videoBitRate;
    std::string audioCodec;
    int64_t audioBitRate;

    static int frameNumber;

    int64_t frameTotalNumber;
};

#endif // TRANSCODERFFTOOL_H
