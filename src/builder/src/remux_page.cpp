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

#include "../include/remux_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include "../include/transcoder_helper.h"
#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

RemuxPage::RemuxPage(QWidget *parent) : BasePage(parent), converterRunner(nullptr) {
    SetupUI();
}

RemuxPage::~RemuxPage() {
}

void RemuxPage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileSelector->GetLineEdit(), outputFileSelector->GetLineEdit(),
                           formatWidget->GetFormat());
}

void RemuxPage::OnInputFileChanged(const QString &newPath) {
    AnalyzeStreams(newPath);
}

void RemuxPage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void RemuxPage::OnPageDeactivated() {
    BasePage::OnPageDeactivated();
}

void RemuxPage::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input File Selector
    inputFileSelector = new FileSelectorWidget(
        tr("Input File"),
        FileSelectorWidget::InputFile,
        tr("Select a media file..."),
        tr("Media Files (*.mp4 *.avi *.mkv *.mov *.flv *.wmv *.webm *.ts *.m4v);;All Files (*.*)"),
        tr("Select Media File"),
        this
    );
    connect(inputFileSelector, &FileSelectorWidget::FileSelected, this, &RemuxPage::OnInputFileSelected);
    mainLayout->addWidget(inputFileSelector);

    // Streams Section
    streamsGroupBox = new QGroupBox(tr("Streams (Select streams to include)"), this);
    QVBoxLayout *streamsGroupLayout = new QVBoxLayout(streamsGroupBox);

    streamsScrollArea = new QScrollArea(streamsGroupBox);
    streamsScrollArea->setWidgetResizable(true);
    streamsScrollArea->setMinimumHeight(150);
    streamsScrollArea->setMaximumHeight(250);

    streamsContainer = new QWidget();
    streamsLayout = new QVBoxLayout(streamsContainer);
    streamsLayout->setSpacing(5);
    streamsLayout->setContentsMargins(5, 5, 5, 5);

    QLabel *noStreamsLabel = new QLabel(tr("No file selected"), streamsContainer);
    noStreamsLabel->setStyleSheet("color: gray; font-style: italic;");
    streamsLayout->addWidget(noStreamsLabel);
    streamsLayout->addStretch();

    streamsContainer->setLayout(streamsLayout);
    streamsScrollArea->setWidget(streamsContainer);

    streamsGroupLayout->addWidget(streamsScrollArea);
    mainLayout->addWidget(streamsGroupBox);

    // Settings Section
    settingsGroupBox = new QGroupBox(tr("Output Settings"), this);
    QGridLayout *settingsLayout = new QGridLayout(settingsGroupBox);
    settingsLayout->setSpacing(10);

    // Output Format
    formatLabel = new QLabel(tr("Output Format:"), this);
    formatWidget = new FormatSelectorWidget(FormatSelectorWidget::Video, false, this);
    formatWidget->SetFormat("mp4");  // Default to mp4
    connect(formatWidget, &FormatSelectorWidget::FormatChanged, this, &RemuxPage::OnFormatChanged);
    settingsLayout->addWidget(formatLabel, 0, 0);
    settingsLayout->addWidget(formatWidget, 0, 1);

    mainLayout->addWidget(settingsGroupBox);

    // Progress Section
    progressWidget = new ProgressWidget(this);
    mainLayout->addWidget(progressWidget);

    // Output File Selector
    outputFileSelector = new FileSelectorWidget(
        tr("Output File"),
        FileSelectorWidget::OutputFile,
        tr("Output file path will be generated automatically..."),
        tr("Media Files (*.mp4 *.mkv *.avi *.mov *.flv *.wmv *.webm *.ts *.m4v);;All Files (*.*)"),
        tr("Save Remuxed File"),
        this
    );
    connect(outputFileSelector, &FileSelectorWidget::FileSelected, this, &RemuxPage::OnOutputFileSelected);
    mainLayout->addWidget(outputFileSelector);

    // Remux Button
    remuxButton = new QPushButton(tr("Remux"), this);
    remuxButton->setEnabled(false);
    remuxButton->setMinimumHeight(40);
    connect(remuxButton, &QPushButton::clicked, this, &RemuxPage::OnRemuxClicked);
    mainLayout->addWidget(remuxButton);

    // Create conversion runner
    converterRunner = new ConverterRunner(
        progressWidget->GetProgressBar(), progressWidget->GetProgressLabel(), remuxButton,
        tr("Remuxing..."), tr("Remux"),
        tr("Success"), tr("File remuxed successfully!"),
        tr("Error"), tr("Failed to remux file."),
        this
    );
    connect(converterRunner, &ConverterRunner::ConversionFinished, this, &RemuxPage::OnRemuxFinished);

    // Set custom validator to check stream selection
    converterRunner->SetValidator([this]() {
        bool hasSelectedStream = false;
        for (const StreamInfo &stream : streams) {
            if (stream.checkbox && stream.checkbox->isChecked()) {
                hasSelectedStream = true;
                break;
            }
        }
        if (!hasSelectedStream) {
            QMessageBox::warning(this, tr("Error"), tr("Please select at least one stream to remux."));
            return false;
        }
        return true;
    });

    mainLayout->addStretch();

    setLayout(mainLayout);
}

void RemuxPage::OnInputFileSelected(const QString &filePath) {
    // Update shared input file path
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetInputFilePath(filePath);
    }

    AnalyzeStreams(filePath);
    UpdateOutputPath();
}

void RemuxPage::OnOutputFileSelected(const QString &filePath) {
    // Mark output path as manually set
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetOutputFilePath(filePath);
    }
}

void RemuxPage::OnFormatChanged(const QString &format) {
    Q_UNUSED(format);
    UpdateOutputPath();
}

void RemuxPage::OnRemuxClicked() {
    QString inputPath = inputFileSelector->GetFilePath();
    QString outputPath = outputFileSelector->GetFilePath();

    // For remuxing, we don't set any codec parameters (copy all streams)
    EncodeParameter *encodeParam = new EncodeParameter();
    ProcessParameter *processParam = new ProcessParameter();

    // Empty codec names mean copy streams without re-encoding
    // This is the standard way to perform remuxing

    // Get current transcoder from main window
    QString transcoderName = TranscoderHelper::GetCurrentTranscoderName(this);

    // Run conversion using ConverterRunner (validator checks stream selection)
    converterRunner->RunConversion(inputPath, outputPath, encodeParam, processParam, transcoderName);
}

void RemuxPage::OnRemuxFinished(bool success) {
    Q_UNUSED(success);
    // ConverterRunner handles all UI updates and message boxes
    // This slot is kept for potential custom post-processing
}

void RemuxPage::UpdateOutputPath() {
    QString inputPath = inputFileSelector->GetFilePath();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString format = formatWidget->GetFormat();
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath(format);
            outputFileSelector->SetFilePath(outputPath);
            remuxButton->setEnabled(true);
        }
    }
}

void RemuxPage::AnalyzeStreams(const QString &filePath) {
    ClearStreams();

    if (filePath.isEmpty()) {
        return;
    }

    AVFormatContext *formatCtx = nullptr;
    QByteArray ba = filePath.toLocal8Bit();

    // Open input file
    if (avformat_open_input(&formatCtx, ba.data(), nullptr, nullptr) < 0) {
        QLabel *errorLabel = new QLabel("Error: Could not open file", streamsContainer);
        errorLabel->setStyleSheet("color: red;");
        streamsLayout->addWidget(errorLabel);
        streamsLayout->addStretch();
        return;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        QLabel *errorLabel = new QLabel("Error: Could not find stream information", streamsContainer);
        errorLabel->setStyleSheet("color: red;");
        streamsLayout->addWidget(errorLabel);
        streamsLayout->addStretch();
        avformat_close_input(&formatCtx);
        return;
    }

    // Iterate through all streams
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        AVStream *stream = formatCtx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;

        StreamInfo streamInfo;
        streamInfo.index = i;
        streamInfo.type = GetStreamTypeName(codecpar->codec_type);
        streamInfo.codec = QString::fromStdString(avcodec_get_name(codecpar->codec_id));

        // Build details string based on stream type
        QStringList detailsList;

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            detailsList << QString("%1x%2").arg(codecpar->width).arg(codecpar->height);
            if (codecpar->bit_rate > 0) {
                detailsList << FormatBitrate(codecpar->bit_rate);
            }
            if (stream->r_frame_rate.num > 0 && stream->r_frame_rate.den > 0) {
                double fps = (double)stream->r_frame_rate.num / stream->r_frame_rate.den;
                detailsList << QString("%1 fps").arg(fps, 0, 'f', 2);
            }
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codecpar->bit_rate > 0) {
                detailsList << FormatBitrate(codecpar->bit_rate);
            }
            detailsList << QString("%1 channels").arg(codecpar->ch_layout.nb_channels);
            if (codecpar->sample_rate > 0) {
                detailsList << QString("%1 Hz").arg(codecpar->sample_rate);
            }
        }

        streamInfo.details = detailsList.join(", ");

        // Create checkbox with stream info
        QString checkboxText = QString("Stream %1: %2, %3")
            .arg(streamInfo.index)
            .arg(streamInfo.type)
            .arg(streamInfo.codec);

        if (!streamInfo.details.isEmpty()) {
            checkboxText += QString(" (%1)").arg(streamInfo.details);
        }

        streamInfo.checkbox = new QCheckBox(checkboxText, streamsContainer);
        streamInfo.checkbox->setChecked(true);  // Select all streams by default

        streamsLayout->addWidget(streamInfo.checkbox);
        streams.append(streamInfo);
    }

    streamsLayout->addStretch();

    // Close the format context
    avformat_close_input(&formatCtx);
}

void RemuxPage::ClearStreams() {
    // Clear existing stream widgets
    while (QLayoutItem *item = streamsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    streams.clear();
}

QString RemuxPage::GetStreamTypeName(int codecType) {
    switch (codecType) {
        case AVMEDIA_TYPE_VIDEO:
            return "Video";
        case AVMEDIA_TYPE_AUDIO:
            return "Audio";
        case AVMEDIA_TYPE_SUBTITLE:
            return "Subtitle";
        case AVMEDIA_TYPE_DATA:
            return "Data";
        case AVMEDIA_TYPE_ATTACHMENT:
            return "Attachment";
        default:
            return "Unknown";
    }
}

QString RemuxPage::FormatBitrate(int64_t bitsPerSec) {
    if (bitsPerSec <= 0) {
        return "Unknown";
    }

    double kbps = bitsPerSec / 1000.0;
    if (kbps < 1000) {
        return QString("%1 kbps").arg(kbps, 0, 'f', 0);
    }

    double mbps = kbps / 1000.0;
    return QString("%1 Mbps").arg(mbps, 0, 'f', 2);
}

void RemuxPage::RetranslateUi() {
    // Update all translatable strings
    inputFileSelector->setTitle(tr("Input File"));
    inputFileSelector->SetPlaceholder(tr("Select a media file..."));
    inputFileSelector->GetBrowseButton()->setText(tr("Browse..."));

    streamsGroupBox->setTitle(tr("Streams (Select streams to include)"));

    settingsGroupBox->setTitle(tr("Output Settings"));
    formatLabel->setText(tr("Output Format:"));
    formatWidget->RetranslateUi();

    outputFileSelector->setTitle(tr("Output File"));
    outputFileSelector->SetPlaceholder(tr("Output file path will be generated automatically..."));
    outputFileSelector->GetBrowseButton()->setText(tr("Browse..."));
    remuxButton->setText(tr("Remux"));
}
