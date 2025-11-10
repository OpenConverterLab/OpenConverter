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

#include "codec_selector_widget.h"
#include <QHBoxLayout>

CodecSelectorWidget::CodecSelectorWidget(CodecType type, QWidget *parent)
    : QWidget(parent), codecType(type), label(nullptr) {
    Initialize(type);
}

CodecSelectorWidget::CodecSelectorWidget(const QString &labelText, CodecType type, QWidget *parent)
    : QWidget(parent), codecType(type) {
    Initialize(type);
    if (label) {
        label->setText(labelText);
    }
}

void CodecSelectorWidget::Initialize(CodecType type) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    // Create combo box
    codecComboBox = new QComboBox(this);
    PopulateCodecs(type);

    connect(codecComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CodecSelectorWidget::OnCodecChanged);

    layout->addWidget(codecComboBox);
    layout->addStretch();
}

void CodecSelectorWidget::PopulateCodecs(CodecType type) {
    codecComboBox->clear();

    if (type == VideoCodec) {
        codecComboBox->addItems({
            "auto",
            "libx264",
            "libx265",
            "libvpx",
            "libvpx-vp9",
            "mpeg4",
            "copy"
        });
    } else {  // AudioCodec
        codecComboBox->addItems({
            "auto",
            "aac",
            "libmp3lame",
            "libvorbis",
            "libopus",
            "copy"
        });
    }

    codecComboBox->setCurrentText("auto");
}

QString CodecSelectorWidget::GetCodec() const {
    QString codec = codecComboBox->currentText();
    return (codec == "auto") ? "" : codec;
}

bool CodecSelectorWidget::IsAuto() const {
    return codecComboBox->currentText() == "auto";
}

bool CodecSelectorWidget::IsCopy() const {
    return codecComboBox->currentText() == "copy";
}

void CodecSelectorWidget::SetCodec(const QString &codec) {
    if (codec.isEmpty()) {
        codecComboBox->setCurrentText("auto");
    } else {
        codecComboBox->setCurrentText(codec);
    }
}

void CodecSelectorWidget::SetToAuto() {
    codecComboBox->setCurrentText("auto");
}

void CodecSelectorWidget::SetCodecList(const QStringList &codecs) {
    QString currentCodec = codecComboBox->currentText();
    codecComboBox->clear();
    codecComboBox->addItems(codecs);

    // Try to restore previous selection
    int index = codecComboBox->findText(currentCodec);
    if (index >= 0) {
        codecComboBox->setCurrentIndex(index);
    }
}

void CodecSelectorWidget::SetEnabled(bool enabled) {
    codecComboBox->setEnabled(enabled);
    if (label) {
        label->setEnabled(enabled);
    }
}

void CodecSelectorWidget::RetranslateUi() {
    // Codec names are not translated (they are technical identifiers)
    // Only the label would be translated, which is handled by the parent page
}

void CodecSelectorWidget::OnCodecChanged(int index) {
    Q_UNUSED(index);
    emit CodecChanged(GetCodec());
}
