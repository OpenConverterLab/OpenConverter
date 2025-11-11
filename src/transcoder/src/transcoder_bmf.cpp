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
TranscoderBMF::TranscoderBMF(ProcessParameter *process_parameter,
                             EncodeParameter *encode_parameter)
    : Transcoder(process_parameter, encode_parameter) {
    frame_total_number = 0;
}

bmf_sdk::CBytes TranscoderBMF::decoder_callback(bmf_sdk::CBytes input) {
    std::string str_info;
    str_info.assign(reinterpret_cast<const char *>(input.buffer), input.size);
    // BMFLOG(BMF_INFO) << "====Callback==== " << str_info;

    std::regex frame_regex(R"(\btotal frame number:\s*(\d+))");
    std::smatch match;

    if (std::regex_search(str_info, match, frame_regex) && match.size() > 1) {
        std::istringstream(match[1]) >> frame_total_number; // Convert to int
        BMFLOG(BMF_DEBUG) << "Extracted Frame Number: " << frame_total_number;
    } else {
        BMFLOG(BMF_WARNING) << "Failed to extract frame number";
    }

    uint8_t bytes[] = {97, 98, 99, 100, 101, 0};
    return bmf_sdk::CBytes{bytes, 6};
}

bmf_sdk::CBytes TranscoderBMF::encoder_callback(bmf_sdk::CBytes input) {
    std::string str_info;
    str_info.assign(reinterpret_cast<const char *>(input.buffer), input.size);
    // BMFLOG(BMF_INFO) << "====Callback==== " << str_info;

    std::regex frame_regex(R"(\bframe number:\s*(\d+))");
    std::smatch match;

    if (std::regex_search(str_info, match, frame_regex) && match.size() > 1) {
        std::istringstream(match[1]) >> frame_number; // Convert to int
        BMFLOG(BMF_DEBUG) << "Extracted Total Frame Number: " << frame_number;

        send_process_parameter(frame_number, frame_total_number);

        if (frame_number == frame_total_number) {
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
    if (encode_parameter->get_video_codec_name() == "copy") {
        copy_video = true;
    } else {
        copy_video = false;
    }

    if (encode_parameter->get_audio_codec_name() == "copy") {
        copy_audio = true;
    } else {
        copy_audio = false;
    }

    nlohmann::json de_video_codec = {"video_codec", ""};
    nlohmann::json de_audio_codec = {"audio_codec", ""};

    if (copy_video) {
        de_video_codec = {"video_codec", "copy"};
    }
    if (copy_audio) {
        de_audio_codec = {"audio_codec", "copy"};
    }

    decoder_para = {
        {"input_path", input_path},
        de_video_codec,
        de_audio_codec,
    };

    // encoder init
    // Build video_params object with only valid parameters
    nlohmann::json video_params = nlohmann::json::object();

    // Always add codec and bitrate
    video_params["codec"] = encode_parameter->get_video_codec_name();
    video_params["bit_rate"] = encode_parameter->get_video_bit_rate();

    // Only add width if it's set (> 0)
    width = encode_parameter->get_width();
    if (width > 0) {
        video_params["width"] = width;
    }

    // Only add height if it's set (> 0)
    height = encode_parameter->get_height();
    if (height > 0) {
        video_params["height"] = height;
    }

    // Only add qscale if it's set (not -1)
    int qscale = encode_parameter->get_qscale();
    if (qscale >= 0) {
        video_params["qscale"] = qscale;
    }

    // Only add pixel format if it's set (not empty)
    std::string pixel_format = encode_parameter->get_pixel_format();
    if (!pixel_format.empty()) {
        video_params["pixel_format"] = pixel_format;
    }

    // Build audio_params object
    nlohmann::json audio_params = nlohmann::json::object();
    audio_params["codec"] = encode_parameter->get_audio_codec_name();
    audio_params["bit_rate"] = encode_parameter->get_audio_bit_rate();

    encoder_para = {{"output_path", output_path},
                    {"video_params", video_params},
                    {"audio_params", audio_params}};
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
