#include "../include/encode_parameter.h"

EncodeParameter::EncodeParameter() {
    videoCodec = "";
    audioCodec = "";

    videoBitRate = 0;
    audioBitRate = 0;

    qscale = -1;

    available = false;
}

void EncodeParameter::set_Qscale(int q) {
    if (q < 0) {
        return;
    }
    qscale = q;
    available = true;
}

int EncodeParameter::get_Qscale() { return qscale; }

bool EncodeParameter::get_Available() { return available; }

void EncodeParameter::set_Video_Codec_Name(std::string vc) {
    if (vc == "") {
        return;
    }
    videoCodec = vc;
    available = true;
}

void EncodeParameter::set_Audio_Codec_Name(std::string ac) {
    if (ac == "") {
        return;
    }
    audioCodec = ac;
    available = true;
}

void EncodeParameter::set_Video_Bit_Rate(int64_t vbr) {
    if (vbr == 0) {
        return;
    }
    videoBitRate = vbr;
    available = true;
}

void EncodeParameter::set_Audio_Bit_Rate(int64_t abr) {
    if (abr == 0) {
        return;
    }
    audioBitRate = abr;
    available = true;
}

std::string EncodeParameter::get_Video_Codec_Name() { return videoCodec; }

std::string EncodeParameter::get_Audio_Codec_Name() { return audioCodec; }

int64_t EncodeParameter::get_Video_Bit_Rate() { return videoBitRate; }

int64_t EncodeParameter::get_Audio_Bit_Rate() { return audioBitRate; }

EncodeParameter::~EncodeParameter() {}
