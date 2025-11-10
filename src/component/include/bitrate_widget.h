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

#ifndef BITRATE_WIDGET_H
#define BITRATE_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSpinBox>

/**
 * @brief Reusable widget for bitrate selection
 *
 * Features:
 * - SpinBox with "auto" (0) support
 * - Configurable range (default: 0-50000 for video, 0-320 for audio)
 * - Unit suffix: " kbps"
 * - Optional label: "Bitrate:"
 *
 * Usage:
 *   BitrateWidget *bitrate = new BitrateWidget(BitrateWidget::Video, this);
 *   bitrate->SetBitrate(5000);  // 5000 kbps
 *   int kbps = bitrate->GetBitrate();  // Returns 0 for "auto"
 */
class BitrateWidget : public QWidget {
    Q_OBJECT

public:
    enum BitrateType {
        Video,  // Range: 0-50000 kbps
        Audio   // Range: 0-320 kbps
    };

    explicit BitrateWidget(BitrateType type = Video, QWidget *parent = nullptr);
    explicit BitrateWidget(const QString &labelText, BitrateType type = Video, QWidget *parent = nullptr);

    // Getters
    int GetBitrate() const;  // Returns bitrate in kbps, 0 for "auto"
    bool IsAuto() const;

    // Setters
    void SetBitrate(int kbps);  // Set bitrate in kbps, 0 for "auto"
    void SetToAuto();
    void SetRange(int min, int max);

    // Enable/disable
    void SetEnabled(bool enabled);

    // Translation support
    void RetranslateUi();

signals:
    void BitrateChanged(int kbps);

private slots:
    void OnValueChanged(int value);

private:
    void SetupUI(const QString &labelText, BitrateType type);

    QLabel *label;
    QSpinBox *bitrateSpinBox;
};

#endif // BITRATE_WIDGET_H
