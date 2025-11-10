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

#include "../include/transcode_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include "../include/batch_queue.h"
#include "../include/batch_item.h"
#include "../include/transcoder_helper.h"
#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>

TranscodePage::TranscodePage(QWidget *parent) : BasePage(parent), converterRunner(nullptr) {
    SetupUI();
}

TranscodePage::~TranscodePage() {
}

void TranscodePage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileSelector->GetLineEdit(), outputFileSelector->GetLineEdit(),
                           formatComboBox->currentText());
}

void TranscodePage::OnInputFileChanged(const QString &newPath) {
    // Set default format to same as input file
    QString ext = GetFileExtension(newPath);
    if (!ext.isEmpty()) {
        int index = formatComboBox->findText(ext);
        if (index >= 0) {
            formatComboBox->setCurrentIndex(index);
        }
    }
}

void TranscodePage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void TranscodePage::OnPageDeactivated() {
    BasePage::OnPageDeactivated();
}

void TranscodePage::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input File Selector (with Batch button)
    inputFileSelector = new FileSelectorWidget(
        tr("Input File"),
        FileSelectorWidget::InputFile,
        tr("Select a media file or click Batch for multiple files..."),
        tr("Media Files (*.mp4 *.avi *.mkv *.mov *.flv *.wmv *.webm *.ts *.m4v);;All Files (*.*)"),
        tr("Select Media File"),
        this
    );
    connect(inputFileSelector, &FileSelectorWidget::FileSelected, this, &TranscodePage::OnInputFileSelected);
    mainLayout->addWidget(inputFileSelector);

    // Video Settings Section
    videoGroupBox = new QGroupBox(tr("Video Settings"), this);
    QGridLayout *videoLayout = new QGridLayout(videoGroupBox);
    videoLayout->setSpacing(10);

    // Video Codec
    videoCodecLabel = new QLabel(tr("Codec:"), videoGroupBox);
    videoCodecComboBox = new QComboBox(videoGroupBox);
    videoCodecComboBox->addItems({"auto", "libx264", "libx265", "libvpx", "libvpx-vp9", "mpeg4", "copy"});
    videoCodecComboBox->setCurrentText("auto");
    connect(videoCodecComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TranscodePage::OnVideoCodecChanged);

    // Video Bitrate
    videoBitrateLabel = new QLabel(tr("Bitrate:"), videoGroupBox);
    videoBitrateWidget = new BitrateWidget(BitrateWidget::Video, videoGroupBox);

    // Resolution
    resolutionLabel = new QLabel(tr("Resolution:"), videoGroupBox);
    resolutionWidget = new ResolutionWidget(videoGroupBox);
    resolutionWidget->SetWidthRange(0, 7680);
    resolutionWidget->SetHeightRange(0, 4320);

    // Pixel Format
    pixelFormatLabel = new QLabel(tr("Pixel Format:"), videoGroupBox);
    pixelFormatWidget = new PixelFormatWidget(PixelFormatWidget::Video, videoGroupBox);

    videoLayout->addWidget(videoCodecLabel, 0, 0);
    videoLayout->addWidget(videoCodecComboBox, 0, 1, 1, 3);
    videoLayout->addWidget(videoBitrateLabel, 1, 0);
    videoLayout->addWidget(videoBitrateWidget, 1, 1, 1, 3);
    videoLayout->addWidget(resolutionLabel, 2, 0);
    videoLayout->addWidget(resolutionWidget, 2, 1, 1, 3);
    videoLayout->addWidget(pixelFormatLabel, 3, 0);
    videoLayout->addWidget(pixelFormatWidget, 3, 1, 1, 3);

    mainLayout->addWidget(videoGroupBox);

    // Audio Settings Section
    audioGroupBox = new QGroupBox(tr("Audio Settings"), this);
    QGridLayout *audioLayout = new QGridLayout(audioGroupBox);
    audioLayout->setSpacing(10);

    // Audio Codec
    audioCodecLabel = new QLabel(tr("Codec:"), audioGroupBox);
    audioCodecComboBox = new QComboBox(audioGroupBox);
    audioCodecComboBox->addItems({"auto", "aac", "libmp3lame", "libvorbis", "libopus", "copy"});
    audioCodecComboBox->setCurrentText("auto");

    // Audio Bitrate
    audioBitrateWidget = new BitrateWidget(BitrateWidget::Audio, audioGroupBox);

    audioLayout->addWidget(audioCodecLabel, 0, 0);
    audioLayout->addWidget(audioCodecComboBox, 0, 1);
    audioLayout->addWidget(new QLabel(tr("Bitrate:"), audioGroupBox), 1, 0);
    audioLayout->addWidget(audioBitrateWidget, 1, 1);

    mainLayout->addWidget(audioGroupBox);

    // Preset Section
    presetGroupBox = new QGroupBox(tr("Preset"), this);
    QHBoxLayout *presetLayout = new QHBoxLayout(presetGroupBox);

    presetLabel = new QLabel(tr("Preset:"), presetGroupBox);
    presetComboBox = new QComboBox(presetGroupBox);
    presetComboBox->addItems({"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"});
    presetComboBox->setCurrentText("medium");

    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(presetComboBox);
    presetLayout->addStretch();

    mainLayout->addWidget(presetGroupBox);

    // Format Section
    formatGroupBox = new QGroupBox(tr("File Format"), this);
    QHBoxLayout *formatLayout = new QHBoxLayout(formatGroupBox);

    formatLabel = new QLabel(tr("Format:"), formatGroupBox);
    formatComboBox = new QComboBox(formatGroupBox);
    formatComboBox->addItems({"mp4", "mkv", "avi", "mov", "flv", "webm", "ts"});
    formatComboBox->setCurrentText("mp4");
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TranscodePage::OnFormatChanged);

    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(formatComboBox);
    formatLayout->addStretch();

    mainLayout->addWidget(formatGroupBox);

    // Output File Selector (for single file mode)
    outputFileSelector = new FileSelectorWidget(
        tr("Output File"),
        FileSelectorWidget::OutputFile,
        tr("Output file path will be generated automatically..."),
        tr("Media Files (*.mp4 *.mkv *.avi *.mov *.flv *.wmv *.webm *.ts *.m4v);;All Files (*.*)"),
        tr("Save Transcoded File"),
        this
    );
    connect(outputFileSelector, &FileSelectorWidget::FileSelected, this, &TranscodePage::OnOutputFileSelected);
    mainLayout->addWidget(outputFileSelector);

    // Batch Output Widget (for batch mode, replaces output file selector)
    batchOutputWidget = new BatchOutputWidget(this);
    batchOutputWidget->setVisible(false);
    mainLayout->addWidget(batchOutputWidget);

    // Transcode Button
    transcodeButton = new QPushButton(tr("Transcode / Add to Queue"), this);
    transcodeButton->setEnabled(false);
    transcodeButton->setMinimumHeight(40);
    connect(transcodeButton, &QPushButton::clicked, this, &TranscodePage::OnTranscodeClicked);
    mainLayout->addWidget(transcodeButton);

    // Progress Section (placed after button to avoid blank space)
    progressWidget = new ProgressWidget(this);
    mainLayout->addWidget(progressWidget);

    // Create conversion runner
    converterRunner = new ConverterRunner(
        progressWidget->GetProgressBar(), progressWidget->GetProgressLabel(), transcodeButton,
        tr("Transcoding..."), tr("Transcode / Add to Queue"),
        tr("Success"), tr("File transcoded successfully!"),
        tr("Error"), tr("Failed to transcode file."),
        this
    );
    connect(converterRunner, &ConverterRunner::ConversionFinished, this, &TranscodePage::OnTranscodeFinished);

    // Create batch mode helper
    batchModeHelper = new BatchModeHelper(
        inputFileSelector, batchOutputWidget, transcodeButton,
        tr("Transcode / Add to Queue"), tr("Add to Queue"), this
    );
    batchModeHelper->SetSingleOutputWidget(outputFileSelector);  // Hide output selector in batch mode
    batchModeHelper->SetEncodeParameterCreator([this]() {
        return CreateEncodeParameter();
    });

    setLayout(mainLayout);
}

void TranscodePage::OnInputFileSelected(const QString &filePath) {
    // Update shared input file path
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetInputFilePath(filePath);
    }

    // Set default format to same as input file
    QString ext = GetFileExtension(filePath);
    if (!ext.isEmpty()) {
        int index = formatComboBox->findText(ext);
        if (index >= 0) {
            formatComboBox->setCurrentIndex(index);
        }
    }

    UpdateOutputPath();
}

void TranscodePage::OnOutputFileSelected(const QString &filePath) {
    // Mark output path as manually set
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetOutputFilePath(filePath);
    }
}

void TranscodePage::OnFormatChanged(int index) {
    Q_UNUSED(index);
    UpdateOutputPath();
}

void TranscodePage::OnVideoCodecChanged(int index) {
    Q_UNUSED(index);
}

void TranscodePage::OnTranscodeClicked() {
    // Check if batch mode is active
    if (batchModeHelper->IsBatchMode()) {
        // Batch mode: Add to queue
        QString format = formatComboBox->currentText();
        batchModeHelper->AddToQueue(format);
        return;
    }

    // Single file mode: Transcode immediately
    QString inputPath = inputFileSelector->GetFilePath();
    QString outputPath = outputFileSelector->GetFilePath();

    // Create encode parameters
    EncodeParameter *encodeParam = CreateEncodeParameter();
    ProcessParameter *processParam = new ProcessParameter();

    // Get current transcoder from main window
    QString transcoderName = TranscoderHelper::GetCurrentTranscoderName(this);

    // Run conversion using ConverterRunner
    converterRunner->RunConversion(inputPath, outputPath, encodeParam, processParam, transcoderName);
}

void TranscodePage::OnTranscodeFinished(bool success) {
    Q_UNUSED(success);
    // ConverterRunner handles all UI updates and message boxes
    // This slot is kept for potential custom post-processing
}

void TranscodePage::UpdateOutputPath() {
    QString inputPath = inputFileSelector->GetFilePath();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString format = formatComboBox->currentText();
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath(format);
            outputFileSelector->SetFilePath(outputPath);
            transcodeButton->setEnabled(true);
        }
    }
}

QString TranscodePage::GetFileExtension(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower();
}

EncodeParameter* TranscodePage::CreateEncodeParameter() {
    EncodeParameter *encodeParam = new EncodeParameter();

    // Video codec
    QString videoCodec = videoCodecComboBox->currentText();
    if (videoCodec != "auto") {
        encodeParam->set_video_codec_name(videoCodec.toStdString());
    }

    // Video bitrate
    int videoBitrate = videoBitrateWidget->GetBitrate();
    if (videoBitrate > 0) {
        encodeParam->set_video_bit_rate(videoBitrate * 1000);
    }

    // Resolution
    int width = resolutionWidget->GetWidth();
    int height = resolutionWidget->GetHeight();
    if (width > 0) {
        encodeParam->set_width(width);
    }
    if (height > 0) {
        encodeParam->set_height(height);
    }

    // Pixel format
    QString pixFmt = pixelFormatWidget->GetPixelFormat();
    if (!pixFmt.isEmpty()) {
        encodeParam->set_pixel_format(pixFmt.toStdString());
    }

    // Audio codec
    QString audioCodec = audioCodecComboBox->currentText();
    if (audioCodec != "auto") {
        encodeParam->set_audio_codec_name(audioCodec.toStdString());
    }

    // Audio bitrate
    int audioBitrate = audioBitrateWidget->GetBitrate();
    if (audioBitrate > 0) {
        encodeParam->set_audio_bit_rate(audioBitrate * 1000);
    }

    // Preset
    QString preset = presetComboBox->currentText();
    if (!preset.isEmpty()) {
        encodeParam->set_preset(preset.toStdString());
    }

    return encodeParam;
}

void TranscodePage::RetranslateUi() {
    // Update all translatable strings
    inputFileSelector->setTitle(tr("Input File"));
    inputFileSelector->SetPlaceholder(tr("Select a media file..."));
    inputFileSelector->RetranslateUi();

    videoGroupBox->setTitle(tr("Video Settings"));
    videoCodecLabel->setText(tr("Codec:"));
    videoBitrateLabel->setText(tr("Bitrate:"));
    videoBitrateWidget->RetranslateUi();
    resolutionLabel->setText(tr("Resolution:"));
    resolutionWidget->RetranslateUi();
    pixelFormatLabel->setText(tr("Pixel Format:"));
    pixelFormatWidget->RetranslateUi();

    audioGroupBox->setTitle(tr("Audio Settings"));
    audioCodecLabel->setText(tr("Codec:"));
    audioBitrateWidget->RetranslateUi();

    presetGroupBox->setTitle(tr("Preset"));
    presetLabel->setText(tr("Preset:"));

    formatGroupBox->setTitle(tr("File Format"));
    formatLabel->setText(tr("Format:"));

    outputFileSelector->setTitle(tr("Output File"));
    outputFileSelector->SetPlaceholder(tr("Output file path will be generated automatically..."));
    outputFileSelector->RetranslateUi();

    // Update batch widgets
    batchOutputWidget->RetranslateUi();

    // Update button text based on batch mode
    if (batchModeHelper) {
        if (inputFileSelector->IsBatchMode()) {
            transcodeButton->setText(tr("Add to Queue"));
        } else {
            transcodeButton->setText(tr("Transcode / Add to Queue"));
        }
    } else {
        transcodeButton->setText(tr("Transcode"));
    }
}
