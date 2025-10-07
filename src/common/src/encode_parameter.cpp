#include "../include/encode_parameter.h"

EncodeParameter::EncodeParameter() {
    videoCodec = "";
    audioCodec = "";

    videoBitRate = 0;
    audioBitRate = 0;

    qscale = -1;

    available = false;
}

void EncodeParameter::set_qscale(int q) {
    if (q < 0) {
        return;
    }
    qscale = q;
    available = true;
}

int EncodeParameter::get_qscale() { return qscale; }

bool EncodeParameter::get_available() { return available; }

void EncodeParameter::set_video_codec_name(std::string vc) {
    if (vc == "") {
        return;
    }
    videoCodec = vc;
    available = true;
}

void EncodeParameter::set_audio_codec_name(std::string ac) {
    if (ac == "") {
        return;
    }
    audioCodec = ac;
    available = true;
}

void EncodeParameter::set_video_bit_rate(int64_t vbr) {
    if (vbr == 0) {
        return;
    }
    videoBitRate = vbr;
    available = true;
}

void EncodeParameter::set_audio_bit_rate(int64_t abr) {
    if (abr == 0) {
        return;
    }
    audioBitRate = abr;
    available = true;
}

std::string EncodeParameter::get_video_codec_name() { return videoCodec; }

std::string EncodeParameter::get_audio_codec_name() { return audioCodec; }

int64_t EncodeParameter::get_video_bit_rate() { return videoBitRate; }

int64_t EncodeParameter::get_audio_bit_rate() { return audioBitRate; }

EncodeParameter::~EncodeParameter() {}
