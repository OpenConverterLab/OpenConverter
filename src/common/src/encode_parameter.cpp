#include "../include/encode_parameter.h"

EncodeParameter::EncodeParameter() {
    videoCodec = "";
    audioCodec = "";

    videoBitRate = 0;
    audioBitRate = 0;

    qscale = -1;
    pixelFormat = "";
    width = 0;
    height = 0;

    available = false;
}

void EncodeParameter::set_qscale(int q) {
    if (q < 0) {
        return;
    }
    qscale = q;
    available = true;
}

void EncodeParameter::set_pixel_format(std::string p) {
    pixelFormat = p;
    available = true;
}

void EncodeParameter::set_width(uint16_t w) {
    width = w;
    available = true;
}

void EncodeParameter::set_height(uint16_t h) {
    height = h;
    available = true;
}

int EncodeParameter::get_qscale() { return qscale; }

std::string EncodeParameter::get_pixel_format() { return pixelFormat; }

uint16_t EncodeParameter::get_width() { return width; }

uint16_t EncodeParameter::get_height() { return height; }

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
