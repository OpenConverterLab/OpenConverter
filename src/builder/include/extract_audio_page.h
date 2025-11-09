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
#include "converter_runner.h"
#include "file_selector_widget.h"
#include "progress_widget.h"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
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
    void RetranslateUi() override;

protected:
    void OnOutputPathUpdate() override;

private slots:
    void OnInputFileSelected(const QString &filePath);
    void OnOutputFileSelected(const QString &filePath);
    void OnFormatChanged(int index);
    void OnExtractClicked();
    void OnExtractFinished(bool success);

signals:
    void ExtractComplete(bool success);

private:
    void SetupUI();
    void UpdateOutputPath();
    QString DetectAudioCodecFromFile(const QString &filePath);
    QString MapCodecToFormat(const QString &codec);

    // Input/Output section
    FileSelectorWidget *inputFileSelector;
    FileSelectorWidget *outputFileSelector;

    // Settings section
    QGroupBox *settingsGroupBox;
    QLabel *formatLabel;
    QComboBox *formatComboBox;
    QLabel *bitrateLabel;
    QSpinBox *bitrateSpinBox;

    // Progress section
    ProgressWidget *progressWidget;

    // Action section
    QPushButton *extractButton;

    // Conversion runner
    ConverterRunner *converterRunner;
};

#endif // EXTRACT_AUDIO_PAGE_H
