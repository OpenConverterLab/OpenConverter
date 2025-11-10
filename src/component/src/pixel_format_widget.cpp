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

#include "pixel_format_widget.h"
#include <QHBoxLayout>

PixelFormatWidget::PixelFormatWidget(FormatType type, QWidget *parent)
    : QWidget(parent) {
    SetupUI("", type);
}

PixelFormatWidget::PixelFormatWidget(const QString &labelText, FormatType type, QWidget *parent)
    : QWidget(parent) {
    SetupUI(labelText, type);
}

void PixelFormatWidget::SetupUI(const QString &labelText, FormatType type) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Optional label
    if (!labelText.isEmpty()) {
        label = new QLabel(labelText, this);
        layout->addWidget(label);
    } else {
        label = nullptr;
    }

    // Format combo box
    formatComboBox = new QComboBox(this);
    PopulateFormats(type);
    layout->addWidget(formatComboBox);

    // Connect signals
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PixelFormatWidget::OnFormatChanged);

    setLayout(layout);
}

void PixelFormatWidget::PopulateFormats(FormatType type) {
    formatComboBox->clear();
    formatComboBox->addItem("auto");

    if (type == Video) {
        // Common video formats
        formatComboBox->addItems({
            "yuv420p", "yuv422p", "yuv444p",
            "rgb24", "bgr24"
        });
    } else {  // Image
        // Image formats with more options
        formatComboBox->addItems({
            // RGB formats
            "rgb24", "rgba", "rgb48be", "rgba64be",
            // YUV formats (limited range)
            "yuv420p", "yuv422p", "yuv444p",
            // YUVJ formats (full range, common for JPEG)
            "yuvj420p", "yuvj422p", "yuvj444p",
            // Grayscale
            "gray", "gray16be",
            // Other common formats
            "bgr24", "bgra"
        });
    }
}

QString PixelFormatWidget::GetPixelFormat() const {
    QString format = formatComboBox->currentText();
    return (format == "auto") ? "" : format;
}

bool PixelFormatWidget::IsAuto() const {
    return formatComboBox->currentText() == "auto";
}

void PixelFormatWidget::SetPixelFormat(const QString &format) {
    if (format.isEmpty() || format == "auto") {
        formatComboBox->setCurrentText("auto");
    } else {
        int index = formatComboBox->findText(format);
        if (index >= 0) {
            formatComboBox->setCurrentIndex(index);
        } else {
            // Format not in list, add it and select it
            formatComboBox->addItem(format);
            formatComboBox->setCurrentText(format);
        }
    }
}

void PixelFormatWidget::SetToAuto() {
    formatComboBox->setCurrentText("auto");
}

void PixelFormatWidget::SetEnabled(bool enabled) {
    formatComboBox->setEnabled(enabled);
    if (label) {
        label->setEnabled(enabled);
    }
}

void PixelFormatWidget::RetranslateUi() {
    // Pixel format names are not translated (they are FFmpeg identifiers)
    // Only the label would be translated, but it's set externally
}

void PixelFormatWidget::OnFormatChanged(int index) {
    Q_UNUSED(index);
    emit PixelFormatChanged(GetPixelFormat());
}
