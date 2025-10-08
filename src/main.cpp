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

namespace fs = std::filesystem;

static bool is_existing_regular_file(const fs::path &p) {
    return fs::exists(p) && fs::is_regular_file(p);
}

static bool is_valid_output_candidate(const fs::path &p) {
    if (!p.has_filename()) return false;        // reject directory-only paths
    fs::path parent = p.parent_path();
    if (parent.empty()) parent = fs::current_path();
    if (fs::exists(p)) return !fs::is_directory(p);        // existing file ok (not a dir)
    return fs::exists(parent) && fs::is_directory(parent); // non-existing file OK if parent dir exists
}

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

bool parseBitrate(const std::string &s, int64_t &out_bps) {

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

static bool confirm_overwrite(const fs::path &p) {
    std::string line;
    while (true) {
        std::cout << "Output file already exists: '" << p.string()
                  << "'. Overwrite? (y/n): " << std::flush;

        if (!std::getline(std::cin, line)) {
            // EOF or error — treat as "no"
            std::cerr << "\nNo response (EOF). Aborting.\n";
            return false;
        }

        if (line == "y" || line == "yes") return true;
        if (line == "n" || line == "no")  return false;

        std::cout << "Please answer 'y' (yes) or 'n' (no).\n";
    }
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
                if (!parseBitrate(argv[++i], videoBitRate)) {
                    std::cerr << "Error: Invalid video bitrate format\n";
                    return false;
                }
            }
        } else if (strcmp(argv[i], "-b:a") == 0 ||
                   strcmp(argv[i], "--bitrate:audio") == 0) {
            if (i + 1 < argc) {
                if (!parseBitrate(argv[++i], audioBitRate)) {
                    std::cerr << "Error: Invalid audio bitrate format\n";
                    return false;
                }
            }
        } else {
            // positional argument: validate as input (existing) or output (candidate)
            fs::path p(argv[i]);

            if (inputFile.empty() && (is_existing_regular_file(p))) {
                inputFile = p.string();
            } else if (outputFile.empty() && is_valid_output_candidate(p) && !inputFile.empty()) {
                if (fs::exists(p))
                    if (!confirm_overwrite(p))
                        return false;
                outputFile = p.string();
            } else {
                // This catches stray tokens like "b" "0" as well as duplicates/ambiguous args
                std::cerr << "Invalid or unexpected argument: '" << argv[i] << "'\n";
                printUsage(argv[0]);
                return false;
            }
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
        encodeParam->set_video_codec_name(videoCodec);
    }
    if (qscale != -1) {
        encodeParam->set_qscale(qscale);
    }
    if (!audioCodec.empty()) {
        encodeParam->set_audio_codec_name(audioCodec);
    }
    if (videoBitRate != -1) {
        encodeParam->set_video_bit_rate(videoBitRate);
    }
    if (audioBitRate != -1) {
        encodeParam->set_audio_bit_rate(audioBitRate);
    }

    // Create converter
    Converter converter(processParam, encodeParam);

    // Set transcoder
    if (!converter.set_transcoder(transcoderType)) {
        std::cerr << "Error: Failed to set transcoder\n";
        delete processParam;
        delete encodeParam;
        return false;
    }

    // Perform conversion
    bool result = converter.convert_format(inputFile, outputFile);
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
