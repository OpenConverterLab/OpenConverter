#include "common/include/encode_parameter.h"
#include "common/include/process_parameter.h"
#include "engine/include/converter.h"
#include <cstring>
#include <iostream>
#include <string>

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
    int videoBitRate = -1;
    int audioBitRate = -1;

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
                videoBitRate = std::stoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-b:a") == 0 ||
                   strcmp(argv[i], "--bitrate:audio") == 0) {
            if (i + 1 < argc) {
                audioBitRate = std::stoi(argv[++i]);
            }
        } else if (inputFile.empty()) {
            inputFile = argv[i];
        } else if (outputFile.empty()) {
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
