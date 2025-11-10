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

#include "resolution_widget.h"
#include <QHBoxLayout>

ResolutionWidget::ResolutionWidget(QWidget *parent)
    : QWidget(parent) {
    SetupUI("");
}

ResolutionWidget::ResolutionWidget(const QString &labelText, QWidget *parent)
    : QWidget(parent) {
    SetupUI(labelText);
}

void ResolutionWidget::SetupUI(const QString &labelText) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Optional label
    if (!labelText.isEmpty()) {
        label = new QLabel(labelText, this);
        layout->addWidget(label);
    } else {
        label = nullptr;
    }

    // Width spin box
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setRange(0, 16384);
    widthSpinBox->setValue(0);
    widthSpinBox->setSpecialValueText(tr("auto"));
    widthSpinBox->setSuffix(tr(" px"));
    layout->addWidget(widthSpinBox);

    // "x" label
    xLabel = new QLabel(tr("x"), this);
    layout->addWidget(xLabel);

    // Height spin box
    heightSpinBox = new QSpinBox(this);
    heightSpinBox->setRange(0, 16384);
    heightSpinBox->setValue(0);
    heightSpinBox->setSpecialValueText(tr("auto"));
    heightSpinBox->setSuffix(tr(" px"));
    layout->addWidget(heightSpinBox);

    // Connect signals
    connect(widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ResolutionWidget::OnWidthChanged);
    connect(heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ResolutionWidget::OnHeightChanged);

    setLayout(layout);
}

int ResolutionWidget::GetWidth() const {
    return widthSpinBox->value();
}

int ResolutionWidget::GetHeight() const {
    return heightSpinBox->value();
}

void ResolutionWidget::SetWidth(int width) {
    widthSpinBox->setValue(width);
}

void ResolutionWidget::SetHeight(int height) {
    heightSpinBox->setValue(height);
}

void ResolutionWidget::SetResolution(int width, int height) {
    widthSpinBox->setValue(width);
    heightSpinBox->setValue(height);
}

void ResolutionWidget::SetRange(int min, int max) {
    widthSpinBox->setRange(min, max);
    heightSpinBox->setRange(min, max);
}

void ResolutionWidget::SetWidthRange(int min, int max) {
    widthSpinBox->setRange(min, max);
}

void ResolutionWidget::SetHeightRange(int min, int max) {
    heightSpinBox->setRange(min, max);
}

void ResolutionWidget::SetEnabled(bool enabled) {
    widthSpinBox->setEnabled(enabled);
    heightSpinBox->setEnabled(enabled);
    if (label) {
        label->setEnabled(enabled);
    }
    xLabel->setEnabled(enabled);
}

void ResolutionWidget::RetranslateUi() {
    widthSpinBox->setSpecialValueText(tr("auto"));
    widthSpinBox->setSuffix(tr(" px"));
    heightSpinBox->setSpecialValueText(tr("auto"));
    heightSpinBox->setSuffix(tr(" px"));
    xLabel->setText(tr("x"));
}

void ResolutionWidget::OnWidthChanged(int value) {
    emit WidthChanged(value);
    emit ResolutionChanged(widthSpinBox->value(), heightSpinBox->value());
}

void ResolutionWidget::OnHeightChanged(int value) {
    emit HeightChanged(value);
    emit ResolutionChanged(widthSpinBox->value(), heightSpinBox->value());
}
