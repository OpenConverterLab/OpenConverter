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

#include "format_selector_widget.h"
#include <QHBoxLayout>

FormatSelectorWidget::FormatSelectorWidget(FormatType type, bool includeAuto, QWidget *parent)
    : QWidget(parent), formatType(type), hasAutoOption(includeAuto), label(nullptr) {
    Initialize(type, includeAuto);
}

FormatSelectorWidget::FormatSelectorWidget(const QString &labelText, FormatType type, bool includeAuto, QWidget *parent)
    : QWidget(parent), formatType(type), hasAutoOption(includeAuto) {
    Initialize(type, includeAuto);
    if (label) {
        label->setText(labelText);
    }
}

void FormatSelectorWidget::Initialize(FormatType type, bool includeAuto) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    // Create combo box
    formatComboBox = new QComboBox(this);
    PopulateFormats(type, includeAuto);

    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FormatSelectorWidget::OnFormatChanged);

    layout->addWidget(formatComboBox);
    layout->addStretch();
}

void FormatSelectorWidget::PopulateFormats(FormatType type, bool includeAuto) {
    formatComboBox->clear();

    QStringList formats;

    if (includeAuto) {
        formats << "auto";
    }

    if (type == Video) {
        formats << "mp4" << "mkv" << "avi" << "mov" << "flv" << "webm" << "ts";
    } else if (type == Audio) {
        formats << "mp3" << "aac" << "wav" << "flac" << "ogg" << "m4a";
    } else {  // Image
        formats << "jpg" << "png" << "bmp" << "webp" << "tiff" << "gif";
    }

    formatComboBox->addItems(formats);

    if (includeAuto) {
        formatComboBox->setCurrentText("auto");
    } else {
        formatComboBox->setCurrentIndex(0);
    }
}

QString FormatSelectorWidget::GetFormat() const {
    QString format = formatComboBox->currentText();
    return (format == "auto") ? "" : format;
}

bool FormatSelectorWidget::IsAuto() const {
    return hasAutoOption && formatComboBox->currentText() == "auto";
}

void FormatSelectorWidget::SetFormat(const QString &format) {
    if (format.isEmpty() && hasAutoOption) {
        formatComboBox->setCurrentText("auto");
    } else {
        formatComboBox->setCurrentText(format);
    }
}

void FormatSelectorWidget::SetToAuto() {
    if (hasAutoOption) {
        formatComboBox->setCurrentText("auto");
    }
}

void FormatSelectorWidget::SetFormatList(const QStringList &formats) {
    QString currentFormat = formatComboBox->currentText();
    formatComboBox->clear();
    formatComboBox->addItems(formats);

    // Try to restore previous selection
    int index = formatComboBox->findText(currentFormat);
    if (index >= 0) {
        formatComboBox->setCurrentIndex(index);
    }
}

void FormatSelectorWidget::SetEnabled(bool enabled) {
    formatComboBox->setEnabled(enabled);
    if (label) {
        label->setEnabled(enabled);
    }
}

void FormatSelectorWidget::RetranslateUi() {
    // Format names are not translated (they are technical identifiers)
    // Only the label would be translated, which is handled by the parent page
}

void FormatSelectorWidget::OnFormatChanged(int index) {
    Q_UNUSED(index);
    emit FormatChanged(GetFormat());
}
