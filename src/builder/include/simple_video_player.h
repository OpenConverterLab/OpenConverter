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

#ifndef SIMPLE_VIDEO_PLAYER_H
#define SIMPLE_VIDEO_PLAYER_H

#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QString>
#include <QThread>
#include <QMutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// Simple FFmpeg-based video player widge
class SimpleVideoPlayer : public QLabel {
    Q_OBJECT

public:
    explicit SimpleVideoPlayer(QWidget *parent = nullptr);
    ~SimpleVideoPlayer() override;

    // Load video file
    bool LoadVideo(const QString &filePath);

    // Playback control
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying() const { return isPlaying; }

    // Seeking
    void Seek(qint64 positionMs);
    qint64 GetPosition() const { return currentPositionMs; }
    qint64 GetDuration() const { return durationMs; }

signals:
    void PositionChanged(qint64 position);
    void DurationChanged(qint64 duration);

private slots:
    void OnPlaybackTimer();

private:
    void CloseVideo();
    bool DecodeNextFrame();
    void DisplayFrame(AVFrame *frame);
    QImage ConvertFrameToQImage(AVFrame *frame);

    // FFmpeg components
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    AVFrame *frame;
    AVFrame *frameRGB;
    AVPacket *packet;
    SwsContext *swsCtx;
    uint8_t *buffer;

    int videoStreamIndex;
    qint64 durationMs;
    qint64 currentPositionMs;
    double timeBase;

    // Playback state
    bool isPlaying;
    bool isLoaded;
    QTimer *playbackTimer;

    // Thread safety
    QMutex mutex;
};

#endif // SIMPLE_VIDEO_PLAYER_H
