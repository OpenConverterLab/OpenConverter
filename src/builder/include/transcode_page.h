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
#include "../../common/include/process_observer.h"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QThread>

class EncodeParameter;
class ProcessParameter;

class TranscodePage : public BasePage, public ProcessObserver {
    Q_OBJECT

public:
    explicit TranscodePage(QWidget *parent = nullptr);
    ~TranscodePage() override;

    void OnPageActivated() override;
    void OnPageDeactivated() override;
    QString GetPageTitle() const override { return "Transcode"; }

    // ProcessObserver interface
    void on_process_update(double progress) override;
    void on_time_update(double timeRequired) override;

protected:
    void OnInputFileChanged(const QString &newPath) override;
    void OnOutputPathUpdate() override;

private slots:
    void OnBrowseInputClicked();
    void OnBrowseOutputClicked();
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
    void RunTranscodeInThread(const QString &inputPath, const QString &outputPath,
                              EncodeParameter *encodeParam, ProcessParameter *processParam);

    // Input section
    QGroupBox *inputGroupBox;
    QLineEdit *inputFileLineEdit;
    QPushButton *browseInputButton;

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
    QProgressBar *progressBar;
    QLabel *progressLabel;

    // Output section
    QGroupBox *outputGroupBox;
    QLineEdit *outputFileLineEdit;
    QPushButton *browseOutputButton;
    QPushButton *transcodeButton;
};

#endif // TRANSCODE_PAGE_H
