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

#ifndef CODEC_SELECTOR_WIDGET_H
#define CODEC_SELECTOR_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QStringList>

/**
 * @brief Reusable widget for codec selection with "auto" option
 *
 * Features:
 * - ComboBox with common codecs
 * - "auto" option (default)
 * - Separate presets for video and audio codecs
 * - Optional label: "Codec:"
 * - Support for "copy" codec (stream copy)
 *
 * Usage:
 *   CodecSelectorWidget *codec = new CodecSelectorWidget(CodecSelectorWidget::Video, this);
 *   codec->SetCodec("libx264");
 *   QString codecName = codec->GetCodec(); // Returns "" for "auto"
 */
class CodecSelectorWidget : public QWidget {
    Q_OBJECT

public:
    enum CodecType {
        VideoCodec,  // Video codecs (libx264, libx265, libvpx, libvpx-vp9, mpeg4, copy)
        AudioCodec   // Audio codecs (aac, libmp3lame, libvorbis, libopus, copy)
    };

    explicit CodecSelectorWidget(CodecType type = VideoCodec, QWidget *parent = nullptr);
    explicit CodecSelectorWidget(const QString &labelText, CodecType type = VideoCodec, QWidget *parent = nullptr);

    // Getters
    QString GetCodec() const;  // Returns "" for "auto"
    bool IsAuto() const;
    bool IsCopy() const;

    // Setters
    void SetCodec(const QString &codec);
    void SetToAuto();
    void SetCodecList(const QStringList &codecs);  // Custom codec list

    // Enable/disable
    void SetEnabled(bool enabled);

    // Translation support
    void RetranslateUi();

signals:
    void CodecChanged(const QString &codec);

private slots:
    void OnCodecChanged(int index);

private:
    void Initialize(CodecType type);
    void PopulateCodecs(CodecType type);

    QLabel *label;
    QComboBox *codecComboBox;
    CodecType codecType;
};

#endif // CODEC_SELECTOR_WIDGET_H
