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

#ifndef TRANSCODE_PAGE_H
#define TRANSCODE_PAGE_H

#include "base_page.h"
#include "converter_runner.h"
#include "file_selector_widget.h"
#include "progress_widget.h"
#include "batch_output_widget.h"
#include "batch_mode_helper.h"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>

class EncodeParameter;
class ProcessParameter;

class TranscodePage : public BasePage {
    Q_OBJECT

public:
    explicit TranscodePage(QWidget *parent = nullptr);
    ~TranscodePage() override;

    void OnPageActivated() override;
    void OnPageDeactivated() override;
    QString GetPageTitle() const override { return "Transcode"; }
    void RetranslateUi() override;

protected:
    void OnInputFileChanged(const QString &newPath) override;
    void OnOutputPathUpdate() override;

private slots:
    void OnInputFileSelected(const QString &filePath);
    void OnOutputFileSelected(const QString &filePath);
    void OnFormatChanged(int index);
    void OnTranscodeClicked();
    void OnVideoCodecChanged(int index);
    void OnTranscodeFinished(bool success);

signals:
    void TranscodeComplete(bool success);

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

    // Video settings section
    QGroupBox *videoGroupBox;
    QLabel *videoCodecLabel;
    QComboBox *videoCodecComboBox;
    QLabel *videoBitrateLabel;
    QSpinBox *videoBitrateSpinBox;
    QLabel *videoBitrateUnitLabel;
    QLabel *dimensionLabel;
    QSpinBox *widthSpinBox;
    QLabel *dimensionXLabel;
    QSpinBox *heightSpinBox;
    QLabel *pixFmtLabel;
    QComboBox *pixFmtComboBox;

    // Audio settings section
    QGroupBox *audioGroupBox;
    QLabel *audioCodecLabel;
    QComboBox *audioCodecComboBox;
    QLabel *audioBitrateLabel;
    QSpinBox *audioBitrateSpinBox;
    QLabel *audioBitrateUnitLabel;

    // Preset section
    QGroupBox *presetGroupBox;
    QLabel *presetLabel;
    QComboBox *presetComboBox;

    // Format section
    QGroupBox *formatGroupBox;
    QLabel *formatLabel;
    QComboBox *formatComboBox;

    // Progress section
    ProgressWidget *progressWidget;

    // Action section
    QPushButton *transcodeButton;

    // Conversion runner
    ConverterRunner *converterRunner;

    // Batch mode helper
    BatchModeHelper *batchModeHelper;
};

#endif // TRANSCODE_PAGE_H
