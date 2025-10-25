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

#ifndef EXTRACT_AUDIO_PAGE_H
#define EXTRACT_AUDIO_PAGE_H

#include "base_page.h"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

class ExtractAudioPage : public BasePage {
    Q_OBJECT

public:
    explicit ExtractAudioPage(QWidget *parent = nullptr);
    ~ExtractAudioPage() override;

    void OnPageActivated() override;
    void OnPageDeactivated() override;
    QString GetPageTitle() const override { return "Extract Audio"; }

private slots:
    void OnBrowseInputClicked();
    void OnBrowseOutputClicked();
    void OnFormatChanged(int index);
    void OnExtractClicked();

private:
    void SetupUI();
    void UpdateOutputPath();

    // Input section
    QGroupBox *inputGroupBox;
    QLineEdit *inputFileLineEdit;
    QPushButton *browseInputButton;

    // Settings section
    QGroupBox *settingsGroupBox;
    QLabel *formatLabel;
    QComboBox *formatComboBox;
    QLabel *codecLabel;
    QComboBox *codecComboBox;
    QLabel *bitrateLabel;
    QSpinBox *bitrateSpinBox;

    // Output section
    QGroupBox *outputGroupBox;
    QLineEdit *outputFileLineEdit;
    QPushButton *browseOutputButton;
    QPushButton *extractButton;
};

#endif // EXTRACT_AUDIO_PAGE_H
