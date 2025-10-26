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

#include "../include/info_view_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include "../../common/include/info.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>

InfoViewPage::InfoViewPage(QWidget *parent) : BasePage(parent) {
    info = new Info();
    SetupUI();
}

InfoViewPage::~InfoViewPage() {
    delete info;
}

QString InfoViewPage::GetPageTitle() const {
    return "Info View";
}

void InfoViewPage::OnPageActivated() {
    BasePage::OnPageActivated();

    // Check if there's a shared input file path
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        QString sharedPath = mainWindow->GetSharedData()->GetInputFilePath();
        // Update if shared path is different from current path
        if (!sharedPath.isEmpty() && sharedPath != filePathLineEdit->text()) {
            filePathLineEdit->setText(sharedPath);
            AnalyzeFile(sharedPath);
        }
    }
}

void InfoViewPage::SetupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input Group
    inputGroupBox = new QGroupBox(tr("Input File"), this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroupBox);

    filePathLineEdit = new QLineEdit(this);
    filePathLineEdit->setPlaceholderText(tr("Drop a media file here or click Browse..."));
    filePathLineEdit->setReadOnly(true);

    browseButton = new QPushButton(tr("Browse..."), this);

    inputLayout->addWidget(filePathLineEdit, 1);
    inputLayout->addWidget(browseButton);

    mainLayout->addWidget(inputGroupBox);

    // Video Info Group
    videoGroupBox = new QGroupBox(tr("Video Information"), this);
    QGridLayout *videoLayout = new QGridLayout(videoGroupBox);
    videoLayout->setColumnStretch(1, 1);

    videoStreamLabel = new QLabel(tr("Stream Index:"), this);
    videoStreamValue = new QLabel("-", this);
    widthLabel = new QLabel(tr("Width:"), this);
    widthValue = new QLabel("-", this);
    heightLabel = new QLabel(tr("Height:"), this);
    heightValue = new QLabel("-", this);
    colorSpaceLabel = new QLabel(tr("Color Space:"), this);
    colorSpaceValue = new QLabel("-", this);
    videoCodecLabel = new QLabel(tr("Video Codec:"), this);
    videoCodecValue = new QLabel("-", this);
    videoBitRateLabel = new QLabel(tr("Bit Rate:"), this);
    videoBitRateValue = new QLabel("-", this);
    frameRateLabel = new QLabel(tr("Frame Rate:"), this);
    frameRateValue = new QLabel("-", this);

    videoLayout->addWidget(videoStreamLabel, 0, 0);
    videoLayout->addWidget(videoStreamValue, 0, 1);
    videoLayout->addWidget(widthLabel, 1, 0);
    videoLayout->addWidget(widthValue, 1, 1);
    videoLayout->addWidget(heightLabel, 2, 0);
    videoLayout->addWidget(heightValue, 2, 1);
    videoLayout->addWidget(colorSpaceLabel, 3, 0);
    videoLayout->addWidget(colorSpaceValue, 3, 1);
    videoLayout->addWidget(videoCodecLabel, 4, 0);
    videoLayout->addWidget(videoCodecValue, 4, 1);
    videoLayout->addWidget(videoBitRateLabel, 5, 0);
    videoLayout->addWidget(videoBitRateValue, 5, 1);
    videoLayout->addWidget(frameRateLabel, 6, 0);
    videoLayout->addWidget(frameRateValue, 6, 1);

    mainLayout->addWidget(videoGroupBox);

    // Audio Info Group
    audioGroupBox = new QGroupBox(tr("Audio Information"), this);
    QGridLayout *audioLayout = new QGridLayout(audioGroupBox);
    audioLayout->setColumnStretch(1, 1);

    audioStreamLabel = new QLabel(tr("Stream Index:"), this);
    audioStreamValue = new QLabel("-", this);
    audioCodecLabel = new QLabel(tr("Audio Codec:"), this);
    audioCodecValue = new QLabel("-", this);
    audioBitRateLabel = new QLabel(tr("Bit Rate:"), this);
    audioBitRateValue = new QLabel("-", this);
    channelsLabel = new QLabel(tr("Channels:"), this);
    channelsValue = new QLabel("-", this);
    sampleFmtLabel = new QLabel(tr("Sample Format:"), this);
    sampleFmtValue = new QLabel("-", this);
    sampleRateLabel = new QLabel(tr("Sample Rate:"), this);
    sampleRateValue = new QLabel("-", this);

    audioLayout->addWidget(audioStreamLabel, 0, 0);
    audioLayout->addWidget(audioStreamValue, 0, 1);
    audioLayout->addWidget(audioCodecLabel, 1, 0);
    audioLayout->addWidget(audioCodecValue, 1, 1);
    audioLayout->addWidget(audioBitRateLabel, 2, 0);
    audioLayout->addWidget(audioBitRateValue, 2, 1);
    audioLayout->addWidget(channelsLabel, 3, 0);
    audioLayout->addWidget(channelsValue, 3, 1);
    audioLayout->addWidget(sampleFmtLabel, 4, 0);
    audioLayout->addWidget(sampleFmtValue, 4, 1);
    audioLayout->addWidget(sampleRateLabel, 5, 0);
    audioLayout->addWidget(sampleRateValue, 5, 1);

    mainLayout->addWidget(audioGroupBox);

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    // Connect signals
    connect(browseButton, &QPushButton::clicked, this, &InfoViewPage::OnBrowseButtonClicked);

    setLayout(mainLayout);
}

void InfoViewPage::OnBrowseButtonClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Media File",
        "",
        "Media Files (*.mp4 *.mkv *.avi *.mov *.flv *.wmv *.mp3 *.wav *.aac *.flac);;All Files (*.*)"
    );

    if (!fileName.isEmpty()) {
        filePathLineEdit->setText(fileName);
        AnalyzeFile(fileName);
    }
}

void InfoViewPage::OnAnalyzeButtonClicked() {
    QString filePath = filePathLineEdit->text();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a media file first.");
        return;
    }

    AnalyzeFile(filePath);
}

void InfoViewPage::HandleFileDrop(const QString &filePath) {
    filePathLineEdit->setText(filePath);
    AnalyzeFile(filePath);
}

void InfoViewPage::AnalyzeFile(const QString &filePath) {
    if (filePath.isEmpty()) {
        return;
    }

    QByteArray ba = filePath.toLocal8Bit();
    char *src = ba.data();

    // Get info by analyzing the file
    info->send_info(src);

    // Display info
    DisplayInfo(info->get_quick_info());

    // Update shared input file path
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetInputFilePath(filePath);
    }
}

void InfoViewPage::DisplayInfo(QuickInfo *quickInfo) {
    if (!quickInfo) {
        ClearInfo();
        return;
    }

    // Video info
    if (quickInfo->videoIdx >= 0) {
        videoStreamValue->setText(QString::number(quickInfo->videoIdx));
        widthValue->setText(QString("%1 px").arg(quickInfo->width));
        heightValue->setText(QString("%1 px").arg(quickInfo->height));
        colorSpaceValue->setText(QString::fromStdString(quickInfo->colorSpace));
        videoCodecValue->setText(QString::fromStdString(quickInfo->videoCodec));
        videoBitRateValue->setText(FormatBitrate(quickInfo->videoBitRate));
        frameRateValue->setText(QString("%1 fps").arg(quickInfo->frameRate, 0, 'f', 2));
    } else {
        videoStreamValue->setText("No video stream");
        widthValue->setText("-");
        heightValue->setText("-");
        colorSpaceValue->setText("-");
        videoCodecValue->setText("-");
        videoBitRateValue->setText("-");
        frameRateValue->setText("-");
    }

    // Audio info
    if (quickInfo->audioIdx >= 0) {
        audioStreamValue->setText(QString::number(quickInfo->audioIdx));
        audioCodecValue->setText(QString::fromStdString(quickInfo->audioCodec));
        audioBitRateValue->setText(FormatBitrate(quickInfo->audioBitRate));
        channelsValue->setText(QString::number(quickInfo->channels));
        sampleFmtValue->setText(QString::fromStdString(quickInfo->sampleFmt));
        sampleRateValue->setText(FormatFrequency(quickInfo->sampleRate));
    } else {
        audioStreamValue->setText("No audio stream");
        audioCodecValue->setText("-");
        audioBitRateValue->setText("-");
        channelsValue->setText("-");
        sampleFmtValue->setText("-");
        sampleRateValue->setText("-");
    }
}

void InfoViewPage::ClearInfo() {
    videoStreamValue->setText("-");
    widthValue->setText("-");
    heightValue->setText("-");
    colorSpaceValue->setText("-");
    videoCodecValue->setText("-");
    videoBitRateValue->setText("-");
    frameRateValue->setText("-");

    audioStreamValue->setText("-");
    audioCodecValue->setText("-");
    audioBitRateValue->setText("-");
    channelsValue->setText("-");
    sampleFmtValue->setText("-");
    sampleRateValue->setText("-");
}

QString InfoViewPage::FormatBitrate(int64_t bitsPerSec) {
    if (bitsPerSec <= 0) {
        return "Unknown";
    }

    double kbps = bitsPerSec / 1000.0;
    if (kbps < 1000) {
        return QString("%1 kbps").arg(kbps, 0, 'f', 2);
    }

    double mbps = kbps / 1000.0;
    return QString("%1 Mbps").arg(mbps, 0, 'f', 2);
}

QString InfoViewPage::FormatFrequency(int64_t hertz) {
    if (hertz <= 0) {
        return "Unknown";
    }

    double khz = hertz / 1000.0;
    return QString("%1 kHz").arg(khz, 0, 'f', 1);
}

void InfoViewPage::RetranslateUi() {
    // Update all translatable strings
    inputGroupBox->setTitle(tr("Input File"));
    filePathLineEdit->setPlaceholderText(tr("Drop a media file here or click Browse..."));
    browseButton->setText(tr("Browse..."));

    videoGroupBox->setTitle(tr("Video Information"));
    videoStreamLabel->setText(tr("Stream Index:"));
    widthLabel->setText(tr("Width:"));
    heightLabel->setText(tr("Height:"));
    colorSpaceLabel->setText(tr("Color Space:"));
    videoCodecLabel->setText(tr("Video Codec:"));
    videoBitRateLabel->setText(tr("Bit Rate:"));
    frameRateLabel->setText(tr("Frame Rate:"));

    audioGroupBox->setTitle(tr("Audio Information"));
    audioStreamLabel->setText(tr("Stream Index:"));
    audioCodecLabel->setText(tr("Audio Codec:"));
    audioBitRateLabel->setText(tr("Bit Rate:"));
    channelsLabel->setText(tr("Channels:"));
    sampleFmtLabel->setText(tr("Sample Format:"));
    sampleRateLabel->setText(tr("Sample Rate:"));
}
