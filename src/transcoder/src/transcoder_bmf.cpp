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
#include <filesystem>
#include <fstream>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <libgen.h>
#endif

/* Receive pointers from converter */
TranscoderBMF::TranscoderBMF(ProcessParameter *process_parameter,
                             EncodeParameter *encode_parameter)
    : Transcoder(process_parameter, encode_parameter) {
    frame_total_number = 0;
}

void TranscoderBMF::setup_python_environment() {
    // In Debug mode, use system PYTHONPATH from environment (set by developer/CMake)
    // In Release mode, set up PYTHONPATH for bundled BMF and Python
#ifndef NDEBUG
    // Debug mode: Set PYTHONPATH based on BMF_ROOT_PATH from environment or CMake
    BMFLOG(BMF_INFO) << "Debug mode: Setting PYTHONPATH from BMF_ROOT_PATH";

    // Get BMF_ROOT_PATH from environment or CMake
    const char* bmf_root_env = std::getenv("BMF_ROOT_PATH");
    std::string bmf_root;

    if (bmf_root_env) {
        bmf_root = std::string(bmf_root_env);
        BMFLOG(BMF_INFO) << "Using BMF_ROOT_PATH from environment: " << bmf_root;
    }
#ifdef BMF_ROOT_PATH_STR
    else {
        bmf_root = BMF_ROOT_PATH_STR;
        BMFLOG(BMF_INFO) << "Using BMF_ROOT_PATH from CMake: " << bmf_root;
    }
#endif

    if (!bmf_root.empty()) {
        // Normalize BMF_ROOT_PATH to include /output/bmf if needed
        if (bmf_root.find("output/bmf") == std::string::npos) {
            bmf_root += "/output/bmf";
        }

        // Set PYTHONPATH: BMF_ROOT_PATH/lib:BMF_ROOT_PATH (parent of /output/bmf)
        std::string bmf_lib_path = bmf_root + "/lib";
        size_t output_pos = bmf_root.find("/output/bmf");
        std::string bmf_output_path;
        if (output_pos != std::string::npos) {
            bmf_output_path = bmf_root.substr(0, output_pos) + "/output";
        } else {
            bmf_output_path = bmf_root;
        }

        // Get existing PYTHONPATH
        std::string current_pythonpath;
        const char* existing_pythonpath = std::getenv("PYTHONPATH");
        if (existing_pythonpath) {
            current_pythonpath = existing_pythonpath;
        }

        // Set PYTHONPATH: bmf/lib:bmf/output:existing
        std::string new_pythonpath = bmf_lib_path + ":" + bmf_output_path;
        if (!current_pythonpath.empty()) {
            new_pythonpath += ":" + current_pythonpath;
        }

        setenv("PYTHONPATH", new_pythonpath.c_str(), 1);
        BMFLOG(BMF_INFO) << "Set PYTHONPATH: " << new_pythonpath;

        // Set BMF_MODULE_CONFIG_PATH
        setenv("BMF_MODULE_CONFIG_PATH", bmf_root.c_str(), 1);
        BMFLOG(BMF_INFO) << "Set BMF_MODULE_CONFIG_PATH: " << bmf_root;
    } else {
        BMFLOG(BMF_WARNING) << "BMF_ROOT_PATH not set. Please set it in environment or CMake.";
        BMFLOG(BMF_WARNING) << "Example: export BMF_ROOT_PATH=/path/to/bmf";
    }

    return;  // Skip bundled BMF setup in Debug mode
#endif

    // Release mode: Set up PYTHONPATH for bundled BMF and Python
    std::string bmf_lib_path;
    std::string bmf_output_path;
    std::string bmf_config_path;
    std::string python_home;
    bool is_bundled = false;

#ifdef __APPLE__
    // Check if running from app bundle
    char exe_path[1024];
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) == 0) {
        std::string exe_dir = std::string(exe_path);
        size_t last_slash = exe_dir.find_last_of('/');
        if (last_slash != std::string::npos) {
            exe_dir = exe_dir.substr(0, last_slash);

            // Check if we're in an app bundle (path contains .app/Contents/MacOS)
            if (exe_dir.find(".app/Contents/MacOS") != std::string::npos) {
                size_t app_pos = exe_dir.find(".app/Contents/MacOS");
                std::string app_bundle = exe_dir.substr(0, app_pos + 4);  // Include .app

                // Check if BMF libraries are actually bundled (Release build)
                std::string bundled_bmf_lib = app_bundle + "/Contents/Frameworks/lib";
                std::string bundled_config = app_bundle + "/Contents/Frameworks/BUILTIN_CONFIG.json";
                std::ifstream bmf_check(bundled_config);

                if (bmf_check.good()) {
                    // BMF libraries are bundled (Release build)
                    bmf_lib_path = bundled_bmf_lib;
                    bmf_output_path = app_bundle + "/Contents/Frameworks";
                    bmf_config_path = app_bundle + "/Contents/Frameworks";
                    BMFLOG(BMF_INFO) << "Using bundled BMF libraries from: " << bmf_lib_path;
                } else {
                    // App bundle exists but BMF not bundled (should not happen in Release)
                    BMFLOG(BMF_WARNING) << "App bundle detected but BMF not bundled";
                }
                bmf_check.close();

                // Check if Python.framework is bundled (Release build)
                std::string python_framework = app_bundle + "/Contents/Frameworks/Python.framework";
                std::ifstream python_check(python_framework + "/Versions/Current/bin/python3");
                if (python_check.good()) {
                    python_home = python_framework + "/Versions/Current";
                    is_bundled = true;
                    BMFLOG(BMF_INFO) << "Using bundled Python from: " << python_home;
                }
                python_check.close();

                // Check for bundled BMF Python package in Resources/bmf_python/
                std::string bundled_bmf_python = app_bundle + "/Contents/Resources/bmf_python";
                std::ifstream bmf_python_check(bundled_bmf_python + "/__init__.py");
                if (bmf_python_check.good()) {
                    // Add bundled BMF Python package to bmf_output_path
                    if (!bmf_output_path.empty()) {
                        bmf_output_path = bundled_bmf_python + ":" + bmf_output_path;
                    } else {
                        bmf_output_path = bundled_bmf_python;
                    }
                    BMFLOG(BMF_INFO) << "Found bundled BMF Python package at: " << bundled_bmf_python;
                }
                bmf_python_check.close();
            }
        }
    }
#endif

    // Set PYTHONHOME if using bundled Python
    if (is_bundled && !python_home.empty()) {
        setenv("PYTHONHOME", python_home.c_str(), 1);
        BMFLOG(BMF_INFO) << "Set PYTHONHOME: " << python_home;

        // Add bundled Python's site-packages to PYTHONPATH
        std::string python_version = "3.9";  // Default, will be detected from bundled Python
        std::string site_packages = python_home + "/lib/python" + python_version + "/site-packages";
        bmf_output_path = site_packages + ":" + bmf_output_path;
    }

    // Get current PYTHONPATH
    std::string current_pythonpath;
    const char* existing_pythonpath = std::getenv("PYTHONPATH");
    if (existing_pythonpath) {
        current_pythonpath = existing_pythonpath;
    }

    // Append BMF paths to PYTHONPATH
    std::string new_pythonpath = bmf_lib_path + ":" + bmf_output_path;
    if (!current_pythonpath.empty()) {
        new_pythonpath += ":" + current_pythonpath;
    }

    // Set PYTHONPATH environment variable
    setenv("PYTHONPATH", new_pythonpath.c_str(), 1);
    BMFLOG(BMF_INFO) << "Set PYTHONPATH: " << new_pythonpath;

    // Set BMF_MODULE_CONFIG_PATH to point to BUILTIN_CONFIG.json
    setenv("BMF_MODULE_CONFIG_PATH", bmf_config_path.c_str(), 1);
    BMFLOG(BMF_INFO) << "Set BMF_MODULE_CONFIG_PATH: " << bmf_config_path;
}

std::string TranscoderBMF::get_python_module_path() {
    std::string module_path;

#ifdef __APPLE__
    // For macOS app bundle
    char exe_path[1024];
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) == 0) {
        char *real_path = realpath(exe_path, nullptr);
        if (real_path) {
            std::filesystem::path exe_dir = std::filesystem::path(real_path).parent_path();
            free(real_path);

            // Check if running from app bundle
            // Path structure: OpenConverter.app/Contents/MacOS/OpenConverter
            if (exe_dir.filename() == "MacOS") {
                std::filesystem::path resources_dir = exe_dir.parent_path() / "Resources" / "modules";
                if (std::filesystem::exists(resources_dir)) {
                    module_path = resources_dir.string();
                    BMFLOG(BMF_INFO) << "Using app bundle module path: " << module_path;
                    return module_path;
                }
            }

            // Check build directory (for development)
            std::filesystem::path build_modules = exe_dir / "modules";
            if (std::filesystem::exists(build_modules)) {
                module_path = build_modules.string();
                BMFLOG(BMF_INFO) << "Using build directory module path: " << module_path;
                return module_path;
            }

            // Check parent directory (for CLI build)
            std::filesystem::path parent_modules = exe_dir.parent_path() / "modules";
            if (std::filesystem::exists(parent_modules)) {
                module_path = parent_modules.string();
                BMFLOG(BMF_INFO) << "Using parent directory module path: " << module_path;
                return module_path;
            }
        }
    }
#else
    // For Linux/Windows
    // Try current directory first
    try {
        std::filesystem::path current_modules = std::filesystem::current_path() / "modules";
        if (std::filesystem::exists(current_modules)) {
            module_path = current_modules.string();
            BMFLOG(BMF_INFO) << "Using current directory module path: " << module_path;
            return module_path;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        BMFLOG(BMF_WARNING) << "Failed to get current directory: " << e.what();
    }
#endif

    // Fallback: use a safe default path
    try {
        module_path = std::filesystem::current_path().string();
        BMFLOG(BMF_WARNING) << "Module path not found, using current directory: " << module_path;
    } catch (const std::filesystem::filesystem_error& e) {
        // If we can't get current directory, use /tmp as last resort
        module_path = "/tmp";
        BMFLOG(BMF_ERROR) << "Failed to get current directory: " << e.what();
        BMFLOG(BMF_ERROR) << "Using fallback path: " << module_path;
    }
    return module_path;
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
        BMFLOG(BMF_WARNING) << "Failed to extract frame number from: " << str_info;
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
    std::string video_codec_name = encode_parameter->get_video_codec_name();
    if (!video_codec_name.empty())
        video_params["codec"] = video_codec_name;
    else
        video_params["codec"] = "libx264";
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
    std::string audio_codec_name = encode_parameter->get_audio_codec_name();
    if (!audio_codec_name.empty())
        audio_params["codec"] = audio_codec_name;
    else
        audio_params["codec"] = "aac";
    audio_params["bit_rate"] = encode_parameter->get_audio_bit_rate();

    encoder_para = {{"output_path", output_path},
                    {"video_params", video_params},
                    {"audio_params", audio_params}};
    return true;
}

bool TranscoderBMF::transcode(std::string input_path, std::string output_path) {
    // Set up Python environment (PYTHONPATH) for BMF Python modules
    setup_python_environment();

    // Set a valid working directory to prevent BMF's internal getcwd() calls from failing
    // When app is launched from Finder, there's no valid current working directory
    try {
        std::filesystem::current_path();  // Test if current path is valid
    } catch (const std::filesystem::filesystem_error& e) {
        // Current directory is invalid, set to a safe location
        try {
#ifdef __APPLE__
            // Try to use app bundle's Resources directory
            char exe_path[1024];
            uint32_t size = sizeof(exe_path);
            if (_NSGetExecutablePath(exe_path, &size) == 0) {
                char *real_path = realpath(exe_path, nullptr);
                if (real_path) {
                    std::filesystem::path exe_dir = std::filesystem::path(real_path).parent_path();
                    free(real_path);

                    // If in app bundle, use Resources directory
                    if (exe_dir.filename() == "MacOS") {
                        std::filesystem::path resources_dir = exe_dir.parent_path() / "Resources";
                        if (std::filesystem::exists(resources_dir)) {
                            std::filesystem::current_path(resources_dir);
                            BMFLOG(BMF_INFO) << "Set working directory to: " << resources_dir.string();
                        } else {
                            // Fallback to /tmp
                            std::filesystem::current_path("/tmp");
                            BMFLOG(BMF_INFO) << "Set working directory to: /tmp";
                        }
                    } else {
                        // Not in app bundle, use executable directory
                        std::filesystem::current_path(exe_dir);
                        BMFLOG(BMF_INFO) << "Set working directory to: " << exe_dir.string();
                    }
                }
            }
#else
            // For Linux/Windows, use /tmp or C:\Temp
            std::filesystem::current_path("/tmp");
            BMFLOG(BMF_INFO) << "Set working directory to: /tmp";
#endif
        } catch (const std::filesystem::filesystem_error& e2) {
            BMFLOG(BMF_ERROR) << "Failed to set working directory: " << e2.what();
            // Continue anyway, BMF might still work
        }
    }

    prepare_info(input_path, output_path);
    int scheduler_cnt = 0;
    AlgoMode algo_mode = encode_parameter->get_algo_mode();
    bmf::builder::Node *algo_node = nullptr;

    auto graph = bmf::builder::Graph(bmf::builder::NormalMode);

    auto decoder =
        graph.Decode(bmf_sdk::JsonParam(decoder_para), "", scheduler_cnt);

    if (algo_mode == AlgoMode::Upscale) {
        int upscale_factor = encode_parameter->get_upscale_factor();
        std::string module_path = get_python_module_path();

        nlohmann::json enhance_option = {
            {"fp32", true},
            {"output_scale", upscale_factor},
        };

        BMFLOG(BMF_INFO) << "Loading enhance module from: " << module_path;

        algo_node = new bmf::builder::Node(
            graph.Module({decoder["video"]}, "", bmf::builder::Python,
                         bmf_sdk::JsonParam(enhance_option), "",
                         module_path,
                         "enhance_module.EnhanceModule",
                         bmf::builder::Immediate,
                         scheduler_cnt));
    }

    auto encoder =
        graph.Encode(algo_mode == AlgoMode::Upscale ? *algo_node : decoder["video"],
                     decoder["audio"],
                     bmf_sdk::JsonParam(encoder_para), "", scheduler_cnt);

    auto de_callback = std::bind(&TranscoderBMF::decoder_callback, this,
                                 std::placeholders::_1);
    auto en_callback = std::bind(&TranscoderBMF::encoder_callback, this,
                                 std::placeholders::_1);

    decoder.AddCallback(
        0, std::function<bmf_sdk::CBytes(bmf_sdk::CBytes)>(de_callback));
    if (algo_mode != AlgoMode::None) {
        algo_node->AddCallback(
            0, std::function<bmf_sdk::CBytes(bmf_sdk::CBytes)>(en_callback));
    } else {
        encoder.AddCallback(
            0, std::function<bmf_sdk::CBytes(bmf_sdk::CBytes)>(en_callback));
    }

    nlohmann::json graph_para = {{"dump_graph", 1}};
    graph.SetOption(bmf_sdk::JsonParam(graph_para));

    int result = graph.Run();

    // Clean up allocated memory
    if (algo_node != nullptr) {
        delete algo_node;
    }

    return (result == 0);
}
