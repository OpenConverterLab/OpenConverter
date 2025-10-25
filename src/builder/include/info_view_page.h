/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
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
