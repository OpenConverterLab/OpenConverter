/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
