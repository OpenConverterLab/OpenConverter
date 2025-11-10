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

#include "quality_widget.h"
#include <QHBoxLayout>

QualityWidget::QualityWidget(QualityType type, QWidget *parent)
    : QWidget(parent), qualityType(type), label(nullptr) {
    Initialize(type);
}

QualityWidget::QualityWidget(const QString &labelText, QualityType type, QWidget *parent)
    : QWidget(parent), qualityType(type) {
    Initialize(type);
    if (label) {
        label->setText(labelText);
    }
}

void QualityWidget::Initialize(QualityType type) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    // Create spin box
    qualitySpinBox = new QSpinBox(this);

    // Set range and default based on type
    if (type == Image) {
        qualitySpinBox->setRange(2, 31);
        qualitySpinBox->setValue(5);
    } else {  // Video
        qualitySpinBox->setRange(18, 28);
        qualitySpinBox->setValue(23);
    }

    connect(qualitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &QualityWidget::OnQualityChanged);

    layout->addWidget(qualitySpinBox);
    layout->addStretch();
}

int QualityWidget::GetQuality() const {
    return qualitySpinBox->value();
}

void QualityWidget::SetQuality(int quality) {
    qualitySpinBox->setValue(quality);
}

void QualityWidget::SetRange(int min, int max) {
    qualitySpinBox->setRange(min, max);
}

void QualityWidget::SetEnabled(bool enabled) {
    qualitySpinBox->setEnabled(enabled);
    if (label) {
        label->setEnabled(enabled);
    }
}

void QualityWidget::RetranslateUi() {
    // Quality widget doesn't have translatable text in the widget itself
    // The label is typically set by the parent page
}

void QualityWidget::OnQualityChanged(int value) {
    emit QualityChanged(value);
}
