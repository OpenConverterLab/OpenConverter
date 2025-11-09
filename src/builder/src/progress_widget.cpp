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

#include "../include/progress_widget.h"

ProgressWidget::ProgressWidget(QWidget *parent) : QWidget(parent) {
    // Create layout
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    // Create progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);

    // Create progress label
    progressLabel = new QLabel("", this);
    progressLabel->setVisible(false);

    // Add to layout
    layout->addWidget(progressBar);
    layout->addWidget(progressLabel);

    setLayout(layout);
}

void ProgressWidget::Show() {
    progressBar->setVisible(true);
    progressLabel->setVisible(true);
}

void ProgressWidget::Hide() {
    progressBar->setVisible(false);
    progressLabel->setVisible(false);
}

void ProgressWidget::Reset() {
    progressBar->setValue(0);
    progressLabel->setText("");
}
