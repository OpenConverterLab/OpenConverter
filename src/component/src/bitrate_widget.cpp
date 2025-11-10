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

#include "bitrate_widget.h"
#include <QHBoxLayout>

BitrateWidget::BitrateWidget(BitrateType type, QWidget *parent)
    : QWidget(parent) {
    SetupUI("", type);
}

BitrateWidget::BitrateWidget(const QString &labelText, BitrateType type, QWidget *parent)
    : QWidget(parent) {
    SetupUI(labelText, type);
}

void BitrateWidget::SetupUI(const QString &labelText, BitrateType type) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Optional label
    if (!labelText.isEmpty()) {
        label = new QLabel(labelText, this);
        layout->addWidget(label);
    } else {
        label = nullptr;
    }

    // Bitrate spin box
    bitrateSpinBox = new QSpinBox(this);
    if (type == Video) {
        bitrateSpinBox->setRange(0, 50000);
    } else {  // Audio
        bitrateSpinBox->setRange(0, 320);
    }
    bitrateSpinBox->setValue(0);
    bitrateSpinBox->setSpecialValueText(tr("auto"));
    bitrateSpinBox->setSuffix(tr(" kbps"));
    layout->addWidget(bitrateSpinBox);

    // Connect signals
    connect(bitrateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BitrateWidget::OnValueChanged);

    setLayout(layout);
}

int BitrateWidget::GetBitrate() const {
    return bitrateSpinBox->value();
}

bool BitrateWidget::IsAuto() const {
    return bitrateSpinBox->value() == 0;
}

void BitrateWidget::SetBitrate(int kbps) {
    bitrateSpinBox->setValue(kbps);
}

void BitrateWidget::SetToAuto() {
    bitrateSpinBox->setValue(0);
}

void BitrateWidget::SetRange(int min, int max) {
    bitrateSpinBox->setRange(min, max);
}

void BitrateWidget::SetEnabled(bool enabled) {
    bitrateSpinBox->setEnabled(enabled);
    if (label) {
        label->setEnabled(enabled);
    }
}

void BitrateWidget::RetranslateUi() {
    bitrateSpinBox->setSpecialValueText(tr("auto"));
    bitrateSpinBox->setSuffix(tr(" kbps"));
}

void BitrateWidget::OnValueChanged(int value) {
    emit BitrateChanged(value);
}
