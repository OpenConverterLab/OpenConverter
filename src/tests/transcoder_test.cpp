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
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

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
    encodeParams.set_video_codec_name("libx264");
    encodeParams.set_video_bit_rate(2000000); // 2Mbps

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

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
    encodeParams.set_video_codec_name(""); // Disable video
    encodeParams.set_audio_codec_name("aac");
    encodeParams.set_audio_bit_rate(128000); // 128kbps

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);
}

// Test for video cutting with copy mode (no re-encoding)
TEST_F(TranscoderTest, VideoCutCopyMode) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output_cut.mp4").string();

    EncodeParameter encodeParams;
    ProcessParameter processParams;

    // Set start and end time (cut first 1 second)
    encodeParams.SetStartTime(0.0);
    encodeParams.SetEndTime(1.0);
    // Leave codecs empty for copy mode

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);

    // Output file should be smaller than input file
    EXPECT_LT(std::filesystem::file_size(outputFile),
              std::filesystem::file_size(inputFile));
}

// Test for video cutting with transcoding
TEST_F(TranscoderTest, VideoCutWithTranscode) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output_cut_transcode.mp4").string();

    EncodeParameter encodeParams;
    ProcessParameter processParams;

    // Set start and end time (cut from 0.5s to 1.5s)
    encodeParams.SetStartTime(0.5);
    encodeParams.SetEndTime(1.5);

    // Set video codec for transcoding
    encodeParams.set_video_codec_name("libx264");
    encodeParams.set_audio_codec_name("aac");

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);
}

// Test for video cutting with invalid time range
TEST_F(TranscoderTest, VideoCutInvalidTimeRange) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output_invalid.mp4").string();

    EncodeParameter encodeParams;
    ProcessParameter processParams;

    // Set invalid time range (end time before start time)
    encodeParams.SetStartTime(2.0);
    encodeParams.SetEndTime(1.0);

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

    // Should fail or produce empty/invalid output
    // Note: Actual behavior depends on FFmpeg implementation
    // This test documents the expected behavior
    if (result) {
        // If it succeeds, output should be very small or empty
        EXPECT_LE(std::filesystem::file_size(outputFile), 1000);
    }
}

// Test for video cutting with only start time
TEST_F(TranscoderTest, VideoCutStartTimeOnly) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output_start_only.mp4").string();

    EncodeParameter encodeParams;
    ProcessParameter processParams;

    // Set only start time (should cut from start time to end of video)
    encodeParams.SetStartTime(0.5);
    // Don't set end time (defaults to -1.0)

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);

    // Output should be smaller than input
    EXPECT_LT(std::filesystem::file_size(outputFile),
              std::filesystem::file_size(inputFile));
}

// Test video cutting with duration (-t option)
TEST_F(TranscoderTest, VideoCutWithDuration) {
    std::string inputFile = (test_dir_ / "test.mp4").string();
    std::string outputFile = (test_dir_ / "output_cut_duration.mp4").string();

    EncodeParameter encodeParams;
    ProcessParameter processParams;

    // Set start time and calculate end time from duration
    // This simulates: -ss 0.5 -t 1.0 (cut from 0.5s for 1 second duration)
    double startTime = 0.5;
    double duration = 1.0;
    encodeParams.SetStartTime(startTime);
    encodeParams.SetEndTime(startTime + duration);  // endTime = 1.5s

    auto converter = std::make_unique<Converter>(&processParams, &encodeParams);
    converter->set_transcoder("FFMPEG");
    bool result = converter->convert_format(inputFile, outputFile);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputFile));
    EXPECT_GT(std::filesystem::file_size(outputFile), 0);

    // Output should be smaller than input
    EXPECT_LT(std::filesystem::file_size(outputFile),
              std::filesystem::file_size(inputFile));
}
