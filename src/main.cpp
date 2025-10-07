#include "common/include/encode_parameter.h"
#include "common/include/process_parameter.h"
#include "engine/include/converter.h"
#include <cstring>
#include <iostream>
#include <string>
#include <filesystem>

#if defined(ENABLE_GUI)
    #include "builder/include/open_converter.h"
    #include <QApplication>
#endif

void printUsage(const char *programName) {
    std::cout << "Usage: " << programName
              << " [options] input_file output_file\n"
              << "Options:\n"
              << "  -t, --transcoder TYPE    Set transcoder type (FFMPEG, BMF, "
                 "FFTOOL)\n"
              << "  -v, --video-codec CODEC  Set video codec\n"
              << "  -q, --qscale QSCALE      Set qscale for video codec\n"
              << "  -a, --audio-codec CODEC  Set audio codec\n"
              << "  -b:v, --bitrate:video BITRATE    Set bitrate for video codec\n"
              << "  -b:a, --bitrate:audio BITRATE    Set bitrate for audio codec\n"
              << "  -h, --help               Show this help message\n";
}

bool parse_bitrate_minimal(const std::string &s, int64_t &out_bps) {

    // split numeric prefix and optional single unit
    size_t pos = 0;
    while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
    if (pos == 0) return false; // no digits

    std::string numpart = s.substr(0, pos);

    // optional single-unit
    uint64_t multiplier = 1;
    if (pos < s.size()) {
        if (pos + 1 != s.size()) return false; // extra chars -> invalid
        char u = s[pos];
        if (u == 'k' || u == 'K') multiplier = 1000ULL;
        else if (u == 'm' || u == 'M') multiplier = 1000ULL * 1000ULL;
        else if (u == 'g' || u == 'G') multiplier = 1000ULL * 1000ULL * 1000ULL;
        else return false; // unknown unit
    }

    // compute safe limit (max value of numeric part)
    uint64_t limit = static_cast<uint64_t>(LLONG_MAX) / multiplier;
    std::string limit_str = std::to_string(limit);

    // strip leading zeros for a fair comparison (leave single zero)
    size_t nz = 0;
    while (nz + 1 < numpart.size() && numpart[nz] == '0') ++nz;
    if (nz) numpart.erase(0, nz);

    // overflow check by length/lexicographic compare
    if (numpart.size() > limit_str.size()) return false;
    if (numpart.size() == limit_str.size() && numpart > limit_str) return false;

    // safe to convert with std::stoull (we validated digits and range)
    unsigned long long value = std::stoull(numpart);
    unsigned long long prod = value * multiplier;

    out_bps = static_cast<int64_t>(prod);
    return true;
}

bool handleCLI(int argc, char *argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return false;
    }

    std::string inputFile;
    std::string outputFile;
    std::string transcoderType = "FFMPEG";
    std::string videoCodec;
    std::string audioCodec;
    int qscale = -1;
    int64_t videoBitRate = -1;
    int64_t audioBitRate = -1;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return false;
        } else if (strcmp(argv[i], "-t") == 0 ||
                   strcmp(argv[i], "--transcoder") == 0) {
            if (i + 1 < argc) {
                transcoderType = argv[++i];
            }
        } else if (strcmp(argv[i], "-v") == 0 ||
                   strcmp(argv[i], "--video-codec") == 0) {
            if (i + 1 < argc) {
                videoCodec = argv[++i];
            }
        } else if (strcmp(argv[i], "-q") == 0 ||
                   strcmp(argv[i], "--qscale") == 0) {
            if (i + 1 < argc) {
                qscale = std::stoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-a") == 0 ||
                   strcmp(argv[i], "--audio-codec") == 0) {
            if (i + 1 < argc) {
                audioCodec = argv[++i];
            }
        } else if (strcmp(argv[i], "-b:v") == 0 ||
                   strcmp(argv[i], "--bitrate:video") == 0) {
            if (i + 1 < argc) {
                if (!parse_bitrate_minimal(argv[++i], videoBitRate)) {
                    std::cerr << "Error: Invalid video bitrate format\n";
                    return false;
                }
            }
        } else if (strcmp(argv[i], "-b:a") == 0 ||
                   strcmp(argv[i], "--bitrate:audio") == 0) {
            if (i + 1 < argc) {
                if (!parse_bitrate_minimal(argv[++i], audioBitRate)) {
                    std::cerr << "Error: Invalid audio bitrate format\n";
                    return false;
                }
            }
        } else {
            if (!std::filesystem::exists(argv[i]) || !std::filesystem::is_regular_file(argv[i]))
                std::cout << "Failed to parse argument: " << argv[i] << "\n";
            else if (inputFile.empty())
                inputFile = argv[i];
            else if (outputFile.empty())
                outputFile = argv[i];
        }
    }

    if (inputFile.empty() || outputFile.empty()) {
        std::cerr << "Error: Input and output files must be specified\n";
        printUsage(argv[0]);
        return false;
    }

    // Create parameters
    ProcessParameter *processParam = new ProcessParameter();
    EncodeParameter *encodeParam = new EncodeParameter();

    // Set codecs if specified
    if (!videoCodec.empty()) {
        encodeParam->set_Video_Codec_Name(videoCodec);
    }
    if (qscale != -1) {
        encodeParam->set_Qscale(qscale);
    }
    if (!audioCodec.empty()) {
        encodeParam->set_Audio_Codec_Name(audioCodec);
    }
    if (videoBitRate != -1) {
        encodeParam->set_Video_Bit_Rate(videoBitRate);
    }
    if (audioBitRate != -1) {
        encodeParam->set_Audio_Bit_Rate(audioBitRate);
    }

    // Create converter
    Converter converter(processParam, encodeParam);

    // Set transcoder
    if (!converter.set_Transcoder(transcoderType)) {
        std::cerr << "Error: Failed to set transcoder\n";
        delete processParam;
        delete encodeParam;
        return false;
    }

    // Perform conversion
    bool result = converter.convert_Format(inputFile, outputFile);
    if (result) {
        std::cout << "Conversion completed successfully\n";
    } else {
        std::cerr << "Conversion failed\n";
    }

    // Cleanup
    delete processParam;
    delete encodeParam;

    return result;
}

int main(int argc, char *argv[]) {
    if (argc > 1)
        return handleCLI(argc, argv) ? 0 : 1;

#if defined(ENABLE_GUI)
    QApplication app(argc, argv);
    OpenConverter w;
    w.show();
    return app.exec();
#endif
    printUsage(argv[0]);
    return 0;
}
