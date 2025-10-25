/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License a
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../include/simple_video_player.h"
#include <QDebug>

SimpleVideoPlayer::SimpleVideoPlayer(QWidget *parent)
    : QLabel(parent),
      formatCtx(nullptr),
      codecCtx(nullptr),
      frame(nullptr),
      frameRGB(nullptr),
      packet(nullptr),
      swsCtx(nullptr),
      buffer(nullptr),
      videoStreamIndex(-1),
      durationMs(0),
      currentPositionMs(0),
      timeBase(0.0),
      isPlaying(false),
      isLoaded(false) {

    setAlignment(Qt::AlignCenter);
    setStyleSheet("background-color: black;");
    setMinimumSize(640, 360);
    setText("No video loaded");

    playbackTimer = new QTimer(this);
    connect(playbackTimer, &QTimer::timeout, this, &SimpleVideoPlayer::OnPlaybackTimer);
}

SimpleVideoPlayer::~SimpleVideoPlayer() {
    CloseVideo();
}

bool SimpleVideoPlayer::LoadVideo(const QString &filePath) {
    QMutexLocker locker(&mutex);

    CloseVideo();

    // Open video file
    if (avformat_open_input(&formatCtx, filePath.toStdString().c_str(), nullptr, nullptr) < 0) {
        qDebug() << "Failed to open video file:" << filePath;
        return false;
    }

    // Retrieve stream information (limit analysis to avoid blocking)
    // Set max_analyze_duration to reduce blocking time
    formatCtx->max_analyze_duration = 5 * AV_TIME_BASE; // 5 seconds max
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        qDebug() << "Failed to find stream info";
        CloseVideo();
        return false;
    }

    // Find video stream
    videoStreamIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (videoStreamIndex < 0) {
        qDebug() << "No video stream found";
        CloseVideo();
        return false;
    }

    AVStream *videoStream = formatCtx->streams[videoStreamIndex];

    // Get duration
    if (formatCtx->duration != AV_NOPTS_VALUE) {
        durationMs = (formatCtx->duration * 1000) / AV_TIME_BASE;
        emit DurationChanged(durationMs);
    }

    // Get time base
    timeBase = av_q2d(videoStream->time_base);

    // Find decoder
    const AVCodec *codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!codec) {
        qDebug() << "Codec not found";
        CloseVideo();
        return false;
    }

    // Allocate codec context
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        qDebug() << "Failed to allocate codec context";
        CloseVideo();
        return false;
    }

    // Copy codec parameters
    if (avcodec_parameters_to_context(codecCtx, videoStream->codecpar) < 0) {
        qDebug() << "Failed to copy codec parameters";
        CloseVideo();
        return false;
    }

    // Open codec
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        qDebug() << "Failed to open codec";
        CloseVideo();
        return false;
    }

    // Allocate frames
    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();
    packet = av_packet_alloc();

    if (!frame || !frameRGB || !packet) {
        qDebug() << "Failed to allocate frames/packet";
        CloseVideo();
        return false;
    }

    // Allocate buffer for RGB frame
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecCtx->width, codecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_RGB24,
                        codecCtx->width, codecCtx->height, 1);

    // Initialize SWS context for color conversion
    swsCtx = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
                           codecCtx->width, codecCtx->height, AV_PIX_FMT_RGB24,
                           SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!swsCtx) {
        qDebug() << "Failed to initialize SWS context";
        CloseVideo();
        return false;
    }

    isLoaded = true;
    currentPositionMs = 0;

    // Decode and display first frame in a deferred manner to avoid blocking
    QTimer::singleShot(0, this, [this]() {
        DecodeNextFrame();
    });

    return true;
}

void SimpleVideoPlayer::Play() {
    if (!isLoaded || isPlaying) {
        return;
    }

    isPlaying = true;
    playbackTimer->start(33); // ~30 fps
}

void SimpleVideoPlayer::Pause() {
    if (!isPlaying) {
        return;
    }

    isPlaying = false;
    playbackTimer->stop();
}

void SimpleVideoPlayer::Stop() {
    Pause();
    Seek(0);
}

void SimpleVideoPlayer::Seek(qint64 positionMs) {
    if (!isLoaded) {
        return;
    }

    QMutexLocker locker(&mutex);

    // Convert position to stream time base
    AVStream *videoStream = formatCtx->streams[videoStreamIndex];
    int64_t timestamp = av_rescale_q(positionMs, AVRational{1, 1000}, videoStream->time_base);

    // Seek to position (use stream index for more accurate seeking)
    if (av_seek_frame(formatCtx, videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        qDebug() << "Seek failed to position:" << positionMs;
        return;
    }

    // Flush codec buffers
    avcodec_flush_buffers(codecCtx);

    // Decode frames until we reach the target position
    bool foundFrame = false;
    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            // Send packet to decoder
            if (avcodec_send_packet(codecCtx, packet) >= 0) {
                // Receive frame from decoder
                int ret = avcodec_receive_frame(codecCtx, frame);
                if (ret == 0) {
                    // Successfully decoded frame
                    if (frame->pts != AV_NOPTS_VALUE) {
                        currentPositionMs = (int64_t)(frame->pts * timeBase * 1000);

                        // Display this frame if it's close to target position
                        if (currentPositionMs >= positionMs - 1000) { // Within 1 second
                            DisplayFrame(frame);
                            foundFrame = true;
                            av_packet_unref(packet);
                            break;
                        }
                    }
                }
            }
        }
        av_packet_unref(packet);
    }

    if (foundFrame) {
        emit PositionChanged(currentPositionMs);
    } else {
        // If we couldn't find a frame, just update position
        currentPositionMs = positionMs;
        emit PositionChanged(currentPositionMs);
    }
}

void SimpleVideoPlayer::OnPlaybackTimer() {
    if (!isPlaying || !isLoaded) {
        return;
    }

    if (DecodeNextFrame()) {
        emit PositionChanged(currentPositionMs);
    } else {
        // End of video
        Pause();
        Seek(0);
    }
}

bool SimpleVideoPlayer::DecodeNextFrame() {
    QMutexLocker locker(&mutex);

    if (!isLoaded) {
        return false;
    }

    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            // Send packet to decoder
            if (avcodec_send_packet(codecCtx, packet) < 0) {
                av_packet_unref(packet);
                continue;
            }

            // Receive frame from decoder
            int ret = avcodec_receive_frame(codecCtx, frame);
            av_packet_unref(packet);

            if (ret == 0) {
                // Successfully decoded frame
                DisplayFrame(frame);

                // Update position
                if (frame->pts != AV_NOPTS_VALUE) {
                    currentPositionMs = (int64_t)(frame->pts * timeBase * 1000);
                }

                return true;
            }
        } else {
            av_packet_unref(packet);
        }
    }

    return false;
}

void SimpleVideoPlayer::DisplayFrame(AVFrame *frame) {
    // Convert frame to RGB
    sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height,
             frameRGB->data, frameRGB->linesize);

    // Convert to QImage and display
    QImage img = ConvertFrameToQImage(frameRGB);
    if (!img.isNull()) {
        setPixmap(QPixmap::fromImage(img).scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

QImage SimpleVideoPlayer::ConvertFrameToQImage(AVFrame *frame) {
    QImage img(frame->data[0], codecCtx->width, codecCtx->height,
              frame->linesize[0], QImage::Format_RGB888);
    return img.copy(); // Deep copy to avoid data corruption
}

void SimpleVideoPlayer::CloseVideo() {
    if (playbackTimer) {
        playbackTimer->stop();
    }

    isPlaying = false;
    isLoaded = false;

    if (swsCtx) {
        sws_freeContext(swsCtx);
        swsCtx = nullptr;
    }

    if (buffer) {
        av_free(buffer);
        buffer = nullptr;
    }

    if (frameRGB) {
        av_frame_free(&frameRGB);
    }

    if (frame) {
        av_frame_free(&frame);
    }

    if (packet) {
        av_packet_free(&packet);
    }

    if (codecCtx) {
        avcodec_free_context(&codecCtx);
    }

    if (formatCtx) {
        avformat_close_input(&formatCtx);
    }

    setText("No video loaded");
}
