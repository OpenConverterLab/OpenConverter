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

#ifndef REMUX_PAGE_H
#define REMUX_PAGE_H

#include "base_page.h"
#include "../../common/include/process_observer.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QThread>
#include <QVBoxLayout>
#include <QVector>

extern "C" {
#include <libavformat/avformat.h>
}

class EncodeParameter;
class ProcessParameter;

struct StreamInfo {
    int index;
    QString type;        // "Video", "Audio", "Subtitle", "Data", "Unknown"
    QString codec;
    QString details;     // Additional info like resolution, bitrate, etc.
    QCheckBox *checkbox;
};

class RemuxPage : public BasePage, public ProcessObserver {
    Q_OBJECT

public:
    explicit RemuxPage(QWidget *parent = nullptr);
    ~RemuxPage() override;

    void OnPageActivated() override;
    void OnPageDeactivated() override;
    QString GetPageTitle() const override { return "Remux"; }

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
    void OnRemuxClicked();
    void OnRemuxFinished(bool success);

signals:
    void RemuxComplete(bool success);

private:
    void SetupUI();
    void UpdateOutputPath();
    void AnalyzeStreams(const QString &filePath);
    void ClearStreams();
    QString GetStreamTypeName(int codecType);
    QString FormatBitrate(int64_t bitsPerSec);
    void RunRemuxInThread(const QString &inputPath, const QString &outputPath,
                          EncodeParameter *encodeParam, ProcessParameter *processParam);

    // Input section
    QGroupBox *inputGroupBox;
    QLineEdit *inputFileLineEdit;
    QPushButton *browseInputButton;

    // Streams section
    QGroupBox *streamsGroupBox;
    QScrollArea *streamsScrollArea;
    QWidget *streamsContainer;
    QVBoxLayout *streamsLayout;
    QVector<StreamInfo> streams;

    // Settings section
    QGroupBox *settingsGroupBox;
    QLabel *formatLabel;
    QComboBox *formatComboBox;

    // Progress section
    QProgressBar *progressBar;
    QLabel *progressLabel;

    // Output section
    QGroupBox *outputGroupBox;
    QLineEdit *outputFileLineEdit;
    QPushButton *browseOutputButton;
    QPushButton *remuxButton;
};

#endif // REMUX_PAGE_H
