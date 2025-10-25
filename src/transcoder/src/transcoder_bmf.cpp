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

#include "../include/transcoder_bmf.h"

/* Receive pointers from converter */
TranscoderBMF::TranscoderBMF(ProcessParameter *processParameter,
                             EncodeParameter *encodeParameter)
    : Transcoder(processParameter, encodeParameter) {
    frameTotalNumber = 0;
}

bmf_sdk::CBytes TranscoderBMF::decoder_callback(bmf_sdk::CBytes input) {
    std::string strInfo;
    strInfo.assign(reinterpret_cast<const char *>(input.buffer), input.size);
    // BMFLOG(BMF_INFO) << "====Callback==== " << strInfo;

    std::regex frame_regex(R"(\btotal frame number:\s*(\d+))");
    std::smatch match;

    if (std::regex_search(strInfo, match, frame_regex) && match.size() > 1) {
        std::istringstream(match[1]) >> frameTotalNumber; // Convert to int
        BMFLOG(BMF_DEBUG) << "Extracted Frame Number: " << frameTotalNumber;
    } else {
        BMFLOG(BMF_WARNING) << "Failed to extract frame number";
    }

    uint8_t bytes[] = {97, 98, 99, 100, 101, 0};
    return bmf_sdk::CBytes{bytes, 6};
}

bmf_sdk::CBytes TranscoderBMF::encoder_callback(bmf_sdk::CBytes input) {
    std::string strInfo;
    strInfo.assign(reinterpret_cast<const char *>(input.buffer), input.size);
    // BMFLOG(BMF_INFO) << "====Callback==== " << strInfo;

    std::regex frame_regex(R"(\bframe number:\s*(\d+))");
    std::smatch match;

    if (std::regex_search(strInfo, match, frame_regex) && match.size() > 1) {
        std::istringstream(match[1]) >> frameNumber; // Convert to int
        BMFLOG(BMF_DEBUG) << "Extracted Total Frame Number: " << frameNumber;

        send_process_parameter(frameNumber, frameTotalNumber);

        if (frameNumber == frameTotalNumber) {
            BMFLOG(BMF_INFO) << "====Callback==== Finish";
        }

    } else {
        BMFLOG(BMF_WARNING) << "Failed to extract frame number";
    }

    uint8_t bytes[] = {97, 98, 99, 100, 101, 0};
    return bmf_sdk::CBytes{bytes, 6};
}

bool TranscoderBMF::prepare_info(std::string input_path,
                                 std::string output_path) {
    // decoder init
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

    nlohmann::json de_video_codec = {"video_codec", ""};
    nlohmann::json de_audio_codec = {"audio_codec", ""};

    if (copyVideo) {
        de_video_codec = {"video_codec", "copy"};
    }
    if (copyAudio) {
        de_audio_codec = {"audio_codec", "copy"};
    }

    decoder_para = {
        {"input_path", input_path},
        de_video_codec,
        de_audio_codec,
    };

    // encoder init
    nlohmann::json en_video_codec = {"codec",
                                     encodeParameter->get_video_codec_name()};
    nlohmann::json en_audio_codec = {"codec",
                                     encodeParameter->get_audio_codec_name()};
    nlohmann::json en_video_bitrate = {"bit_rate",
                                       encodeParameter->get_video_bit_rate()};
    nlohmann::json en_audio_bitrate = {"bit_rate",
                                       encodeParameter->get_audio_bit_rate()};

    encoder_para = {{"output_path", output_path},
                    {"video_params", {en_video_codec, en_video_bitrate}},
                    {"audio_params", {en_audio_codec, en_audio_bitrate}}};
    return true;
}

bool TranscoderBMF::transcode(std::string input_path, std::string output_path) {

    prepare_info(input_path, output_path);
    int scheduler_cnt = 0;

    auto graph = bmf::builder::Graph(bmf::builder::NormalMode);

    auto decoder =
        graph.Decode(bmf_sdk::JsonParam(decoder_para), "", scheduler_cnt++);

    auto encoder =
        graph.Encode(decoder["video"], decoder["audio"],
                     bmf_sdk::JsonParam(encoder_para), "", scheduler_cnt++);

    auto de_callback = std::bind(&TranscoderBMF::decoder_callback, this,
                                 std::placeholders::_1);
    auto en_callback = std::bind(&TranscoderBMF::encoder_callback, this,
                                 std::placeholders::_1);

    decoder.AddCallback(
        0, std::function<bmf_sdk::CBytes(bmf_sdk::CBytes)>(de_callback));
    encoder.AddCallback(
        0, std::function<bmf_sdk::CBytes(bmf_sdk::CBytes)>(en_callback));

    nlohmann::json graph_para = {{"dump_graph", 1}};
    graph.SetOption(bmf_sdk::JsonParam(graph_para));

    if (graph.Run() == 0) {
        return true;
    } else {
        return false;
    }
}
