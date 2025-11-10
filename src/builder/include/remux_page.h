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
#include "converter_runner.h"
#include "file_selector_widget.h"
#include "progress_widget.h"
#include "format_selector_widget.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
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

class RemuxPage : public BasePage {
    Q_OBJECT

public:
    explicit RemuxPage(QWidget *parent = nullptr);
    ~RemuxPage() override;

    void OnPageActivated() override;
    void OnPageDeactivated() override;
    QString GetPageTitle() const override { return "Remux"; }
    void RetranslateUi() override;

protected:
    void OnInputFileChanged(const QString &newPath) override;
    void OnOutputPathUpdate() override;

private slots:
    void OnInputFileSelected(const QString &filePath);
    void OnOutputFileSelected(const QString &filePath);
    void OnFormatChanged(const QString &format);
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

    // Input/Output section
    FileSelectorWidget *inputFileSelector;
    FileSelectorWidget *outputFileSelector;

    // Streams section
    QGroupBox *streamsGroupBox;
    QScrollArea *streamsScrollArea;
    QWidget *streamsContainer;
    QVBoxLayout *streamsLayout;
    QVector<StreamInfo> streams;

    // Settings section
    QGroupBox *settingsGroupBox;
    QLabel *formatLabel;
    FormatSelectorWidget *formatWidget;

    // Progress section
    ProgressWidget *progressWidget;

    // Action section
    QPushButton *remuxButton;

    // Conversion runner
    ConverterRunner *converterRunner;
};

#endif // REMUX_PAGE_H
