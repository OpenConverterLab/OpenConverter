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

#ifndef FORMAT_SELECTOR_WIDGET_H
#define FORMAT_SELECTOR_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QStringList>

/**
 * @brief Reusable widget for output format selection
 *
 * Features:
 * - ComboBox with common formats
 * - Optional "auto" option
 * - Separate presets for video, audio, and image formats
 * - Optional label: "Output Format:"
 *
 * Usage:
 *   FormatSelectorWidget *format = new FormatSelectorWidget(FormatSelectorWidget::Video, this);
 *   format->SetFormat("mp4");
 *   QString formatName = format->GetFormat();
 */
class FormatSelectorWidget : public QWidget {
    Q_OBJECT

public:
    enum FormatType {
        Video,      // Video formats (mp4, mkv, avi, mov, flv, webm, ts)
        Audio,      // Audio formats (mp3, aac, wav, flac, ogg, m4a)
        Image       // Image formats (jpg, png, bmp, webp, tiff, gif)
    };

    explicit FormatSelectorWidget(FormatType type = Video, bool includeAuto = false, QWidget *parent = nullptr);
    explicit FormatSelectorWidget(const QString &labelText, FormatType type = Video, bool includeAuto = false, QWidget *parent = nullptr);

    // Getters
    QString GetFormat() const;  // Returns "" for "auto" if includeAuto is true
    bool IsAuto() const;

    // Setters
    void SetFormat(const QString &format);
    void SetToAuto();
    void SetFormatList(const QStringList &formats);  // Custom format list

    // Enable/disable
    void SetEnabled(bool enabled);

    // Translation support
    void RetranslateUi();

signals:
    void FormatChanged(const QString &format);

private slots:
    void OnFormatChanged(int index);

private:
    void Initialize(FormatType type, bool includeAuto);
    void PopulateFormats(FormatType type, bool includeAuto);

    QLabel *label;
    QComboBox *formatComboBox;
    FormatType formatType;
    bool hasAutoOption;
};

#endif // FORMAT_SELECTOR_WIDGET_H
