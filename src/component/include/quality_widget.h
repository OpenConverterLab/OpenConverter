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

#ifndef QUALITY_WIDGET_H
#define QUALITY_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSpinBox>

/**
 * @brief Reusable widget for quality/qscale selection
 *
 * Features:
 * - SpinBox for quality value (qscale)
 * - Configurable range (default: 2-31 for images, 18-28 for video)
 * - Lower value = better quality
 * - Optional label with description
 *
 * Usage:
 *   QualityWidget *quality = new QualityWidget(QualityWidget::Image, this);
 *   quality->SetQuality(5);
 *   int qscale = quality->GetQuality();
 */
class QualityWidget : public QWidget {
    Q_OBJECT

public:
    enum QualityType {
        Image,  // Image quality (2-31, default: 5)
        Video   // Video quality (18-28, default: 23)
    };

    explicit QualityWidget(QualityType type = Image, QWidget *parent = nullptr);
    explicit QualityWidget(const QString &labelText, QualityType type = Image, QWidget *parent = nullptr);

    // Getters
    int GetQuality() const;

    // Setters
    void SetQuality(int quality);
    void SetRange(int min, int max);

    // Enable/disable
    void SetEnabled(bool enabled);

    // Translation support
    void RetranslateUi();

signals:
    void QualityChanged(int quality);

private slots:
    void OnQualityChanged(int value);

private:
    void Initialize(QualityType type);

    QLabel *label;
    QSpinBox *qualitySpinBox;
    QualityType qualityType;
};

#endif // QUALITY_WIDGET_H
