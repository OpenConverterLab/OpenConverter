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

#ifndef INFO_VIEW_PAGE_H
#define INFO_VIEW_PAGE_H

#include "base_page.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class Info;
class QuickInfo;

class InfoViewPage : public BasePage {
    Q_OBJECT

public:
    explicit InfoViewPage(QWidget *parent = nullptr);
    ~InfoViewPage();

    QString GetPageTitle() const override;
    void OnPageActivated() override;
    void RetranslateUi() override;

    // Handle file drop
    void HandleFileDrop(const QString &filePath);

private slots:
    void OnBrowseButtonClicked();
    void OnAnalyzeButtonClicked();

private:
    void SetupUI();
    void AnalyzeFile(const QString &filePath);
    void DisplayInfo(QuickInfo *quickInfo);
    void ClearInfo();
    QString FormatBitrate(int64_t bitsPerSec);
    QString FormatFrequency(int64_t hertz);

    // UI Components
    QVBoxLayout *mainLayout;
    QGroupBox *inputGroupBox;
    QLineEdit *filePathLineEdit;
    QPushButton *browseButton;

    QGroupBox *videoGroupBox;
    QLabel *videoStreamLabel;
    QLabel *videoStreamValue;
    QLabel *widthLabel;
    QLabel *widthValue;
    QLabel *heightLabel;
    QLabel *heightValue;
    QLabel *colorSpaceLabel;
    QLabel *colorSpaceValue;
    QLabel *videoCodecLabel;
    QLabel *videoCodecValue;
    QLabel *pixelFormatLabel;
    QLabel *pixelFormatValue;
    QLabel *videoBitRateLabel;
    QLabel *videoBitRateValue;
    QLabel *frameRateLabel;
    QLabel *frameRateValue;

    QGroupBox *audioGroupBox;
    QLabel *audioStreamLabel;
    QLabel *audioStreamValue;
    QLabel *audioCodecLabel;
    QLabel *audioCodecValue;
    QLabel *audioBitRateLabel;
    QLabel *audioBitRateValue;
    QLabel *channelsLabel;
    QLabel *channelsValue;
    QLabel *sampleFmtLabel;
    QLabel *sampleFmtValue;
    QLabel *sampleRateLabel;
    QLabel *sampleRateValue;

    // Backend
    Info *info;
};

#endif // INFO_VIEW_PAGE_H
