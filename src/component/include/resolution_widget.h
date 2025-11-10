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

#ifndef RESOLUTION_WIDGET_H
#define RESOLUTION_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSpinBox>

/**
 * @brief Reusable widget for resolution (width x height) selection
 *
 * Features:
 * - Width and Height spin boxes with "auto" (0) support
 * - Configurable range (default: 0-16384)
 * - "x" label between width and height
 * - Horizontal layout: [Width SpinBox] x [Height SpinBox]
 * - Optional label: "Dimension:" or "Resolution:"
 *
 * Usage:
 *   ResolutionWidget *resolution = new ResolutionWidget(this);
 *   resolution->SetResolution(1920, 1080);
 *   int width = resolution->GetWidth();   // Returns 0 for "auto"
 *   int height = resolution->GetHeight(); // Returns 0 for "auto"
 */
class ResolutionWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResolutionWidget(QWidget *parent = nullptr);
    explicit ResolutionWidget(const QString &labelText, QWidget *parent = nullptr);

    // Getters
    int GetWidth() const;
    int GetHeight() const;

    // Setters
    void SetWidth(int width);
    void SetHeight(int height);
    void SetResolution(int width, int height);
    void SetRange(int min, int max);
    void SetWidthRange(int min, int max);
    void SetHeightRange(int min, int max);

    // Enable/disable
    void SetEnabled(bool enabled);

    // Translation support
    void RetranslateUi();

signals:
    void ResolutionChanged(int width, int height);
    void WidthChanged(int width);
    void HeightChanged(int height);

private slots:
    void OnWidthChanged(int value);
    void OnHeightChanged(int value);

private:
    void SetupUI(const QString &labelText);

    QLabel *label;          // Optional label (e.g., "Dimension:")
    QSpinBox *widthSpinBox;
    QLabel *xLabel;         // "x" between width and height
    QSpinBox *heightSpinBox;
};

#endif // RESOLUTION_WIDGET_H
