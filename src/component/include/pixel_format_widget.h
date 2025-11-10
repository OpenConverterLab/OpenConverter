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

#ifndef PIXEL_FORMAT_WIDGET_H
#define PIXEL_FORMAT_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>

/**
 * @brief Reusable widget for pixel format selection
 *
 * Features:
 * - ComboBox with common pixel formats
 * - "auto" option (default)
 * - Separate presets for video and image formats
 * - Optional label: "Pixel Format:"
 *
 * Usage:
 *   PixelFormatWidget *pixFmt = new PixelFormatWidget(PixelFormatWidget::Video, this);
 *   pixFmt->SetPixelFormat("yuv420p");
 *   QString format = pixFmt->GetPixelFormat(); // Returns "" for "auto"
 */
class PixelFormatWidget : public QWidget {
    Q_OBJECT

public:
    enum FormatType {
        Video,  // Common video formats (yuv420p, yuv422p, yuv444p, rgb24, bgr24)
        Image   // Image formats (rgb24, rgba, yuv420p, yuv422p, yuv444p, yuvj420p, gray, etc.)
    };

    explicit PixelFormatWidget(FormatType type = Video, QWidget *parent = nullptr);
    explicit PixelFormatWidget(const QString &labelText, FormatType type = Video, QWidget *parent = nullptr);

    // Getters
    QString GetPixelFormat() const;  // Returns "" for "auto"
    bool IsAuto() const;

    // Setters
    void SetPixelFormat(const QString &format);
    void SetToAuto();

    // Enable/disable
    void SetEnabled(bool enabled);

    // Translation support
    void RetranslateUi();

signals:
    void PixelFormatChanged(const QString &format);

private slots:
    void OnFormatChanged(int index);

private:
    void SetupUI(const QString &labelText, FormatType type);
    void PopulateFormats(FormatType type);

    QLabel *label;
    QComboBox *formatComboBox;
};

#endif // PIXEL_FORMAT_WIDGET_H
