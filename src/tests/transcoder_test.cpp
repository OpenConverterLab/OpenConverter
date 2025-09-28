#include "../common/include/encode_parameter.h"
#include "../engine/include/converter.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>

// Test fixture for transcoder tests
class TranscoderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        test_dir_ = std::filesystem::temp_directory_path() / "transcoder_test";
        std::filesystem::create_directories(test_dir_);

        // Copy test media file to test directory
        std::string test_file = std::string(TEST_MEDIA_PATH) + "/test.mp4";
        if (std::filesystem::exists(test_file)) {
            std::filesystem::copy_file(test_file, test_dir_ / "test.mp4");
        } else {
            FAIL() << "Test media file not found at: " << test_file;
        }
    }

    void TearDown() override {
        // Clean up test directory
        std::filesystem::remove_all(test_dir_);
    }

    std::filesystem::path test_dir_;
};

// Test for remuxing
TEST_F(TranscoderTest, Remux) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output.mp4").string();

    // For remuxing, don't set any codec parameters
    EncodeParameter encodeParams;
    ProcessParameter processParams;

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_Transcoder("FFMPEG");
    bool result = converter->convert_Format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);
}

// Test for video transcoding
TEST_F(TranscoderTest, VideoTranscode) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output.mp4").string();

    EncodeParameter encodeParams;
    ProcessParameter processParams;

    // Set video encoding parameters
    encodeParams.set_Video_Codec_Name("libx264");
    encodeParams.set_Video_Bit_Rate(2000000); // 2Mbps

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_Transcoder("FFMPEG");
    bool result = converter->convert_Format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);
}

// Test for audio transcoding
TEST_F(TranscoderTest, AudioTranscode) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output.aac").string();

    EncodeParameter encodeParams;
    ProcessParameter processParams;

    // Set audio encoding parameters
    encodeParams.set_Video_Codec_Name(""); // Disable video
    encodeParams.set_Audio_Codec_Name("aac");
    encodeParams.set_Audio_Bit_Rate(128000); // 128kbps

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_Transcoder("FFMPEG");
    bool result = converter->convert_Format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);
}
