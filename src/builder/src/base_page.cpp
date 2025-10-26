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

#include "../include/base_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include <QLineEdit>

BasePage::BasePage(QWidget *parent) : QWidget(parent) {}

BasePage::~BasePage() {}

void BasePage::OnPageActivated() {
    // Default implementation - can be overridden by derived classes
}

void BasePage::OnPageDeactivated() {
    // Default implementation - can be overridden by derived classes
}

void BasePage::HandleSharedDataUpdate(QLineEdit *inputFileLineEdit,
                                       QLineEdit *outputFileLineEdit,
                                       const QString &defaultFormat) {
    if (!inputFileLineEdit || !outputFileLineEdit) {
        return;
    }

    // Get shared data from main window
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (!mainWindow || !mainWindow->GetSharedData()) {
        return;
    }

    SharedData *sharedData = mainWindow->GetSharedData();
    QString sharedPath = sharedData->GetInputFilePath();

    if (sharedPath.isEmpty()) {
        return;
    }

    // Check if shared path is different from current input
    if (sharedPath != inputFileLineEdit->text()) {
        // Update input field
        inputFileLineEdit->setText(sharedPath);

        // Call hook for derived class to handle input change
        OnInputFileChanged(sharedPath);

        // Reset output path to auto-generate when input changes
        sharedData->ResetOutputPath();

        // Call hook for derived class to update output path
        OnOutputPathUpdate();
    } else {
        // Input path is the same, but update output path in case format changed
        QString format = defaultFormat;
        QString outputPath = sharedData->GenerateOutputPath(format);
        outputFileLineEdit->setText(outputPath);
    }
}

void BasePage::OnInputFileChanged(const QString &newPath) {
    // Default implementation - can be overridden by derived classes
    Q_UNUSED(newPath);
}

void BasePage::OnOutputPathUpdate() {
    // Default implementation - can be overridden by derived classes
}

void BasePage::RetranslateUi() {
    // Default implementation - can be overridden by derived classes
}
