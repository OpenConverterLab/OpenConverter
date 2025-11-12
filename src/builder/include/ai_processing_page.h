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

#ifndef AI_PROCESSING_PAGE_H
#define AI_PROCESSING_PAGE_H

#include "base_page.h"
#include "converter_runner.h"
#include "file_selector_widget.h"
#include "progress_widget.h"
#include "batch_output_widget.h"
#include "batch_mode_helper.h"
#include "bitrate_widget.h"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>

class EncodeParameter;
class ProcessParameter;

class AIProcessingPage : public BasePage {
    Q_OBJECT

public:
    explicit AIProcessingPage(QWidget *parent = nullptr);
    ~AIProcessingPage() override;

    void OnPageActivated() override;
    void OnPageDeactivated() override;
    QString GetPageTitle() const override { return "AI Processing"; }
    void RetranslateUi() override;

protected:
    void OnInputFileChanged(const QString &newPath) override;
    void OnOutputPathUpdate() override;

private slots:
    void OnInputFileSelected(const QString &filePath);
    void OnOutputFileSelected(const QString &filePath);
    void OnAlgorithmChanged(int index);
    void OnProcessClicked();
    void OnProcessFinished(bool success);

signals:
    void ProcessComplete(bool success);

private:
    void SetupUI();
    void UpdateOutputPath();
    QString GetFileExtension(const QString &filePath);
    EncodeParameter* CreateEncodeParameter();

    // Input/Output section
    FileSelectorWidget *inputFileSelector;
    FileSelectorWidget *outputFileSelector;

    // Batch output widget (shown when batch files selected)
    BatchOutputWidget *batchOutputWidget;

    // Algorithm selection section
    QGroupBox *algorithmGroupBox;
    QLabel *algorithmLabel;
    QComboBox *algorithmComboBox;

    // Algorithm settings section (dynamic based on selected algorithm)
    QGroupBox *algoSettingsGroupBox;
    QStackedWidget *algoSettingsStack;

    // Upscaler settings widget
    QWidget *upscalerSettingsWidget;
    QLabel *upscaleFactorLabel;
    QSpinBox *upscaleFactorSpinBox;

    // Video settings section
    QGroupBox *videoGroupBox;
    QLabel *videoCodecLabel;
    QComboBox *videoCodecComboBox;
    QLabel *videoBitrateLabel;
    BitrateWidget *videoBitrateWidget;

    // Audio settings section
    QGroupBox *audioGroupBox;
    QLabel *audioCodecLabel;
    QComboBox *audioCodecComboBox;
    QLabel *audioBitrateLabel;
    BitrateWidget *audioBitrateWidget;

    // Progress section
    ProgressWidget *progressWidget;

    // Action section
    QPushButton *processButton;

    // Conversion runner
    ConverterRunner *converterRunner;

    // Batch mode helper
    BatchModeHelper *batchModeHelper;
};

#endif // AI_PROCESSING_PAGE_H
