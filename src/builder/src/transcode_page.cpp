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
#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>

TranscodePage::TranscodePage(QWidget *parent) : BasePage(parent) {
    SetupUI();
    connect(this, &TranscodePage::TranscodeComplete, this, &TranscodePage::OnTranscodeFinished);
}

TranscodePage::~TranscodePage() {
}

void TranscodePage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileLineEdit, outputFileLineEdit,
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

    // Input File Section
    inputGroupBox = new QGroupBox("Input File", this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroupBox);

    inputFileLineEdit = new QLineEdit(inputGroupBox);
    inputFileLineEdit->setPlaceholderText("Select a media file...");
    inputFileLineEdit->setReadOnly(true);

    browseInputButton = new QPushButton("Browse...", inputGroupBox);
    connect(browseInputButton, &QPushButton::clicked, this, &TranscodePage::OnBrowseInputClicked);

    inputLayout->addWidget(inputFileLineEdit);
    inputLayout->addWidget(browseInputButton);

    mainLayout->addWidget(inputGroupBox);

    // Video Settings Section
    videoGroupBox = new QGroupBox("Video Settings", this);
    QGridLayout *videoLayout = new QGridLayout(videoGroupBox);
    videoLayout->setSpacing(10);

    // Video Codec
    videoCodecLabel = new QLabel("Codec:", videoGroupBox);
    videoCodecComboBox = new QComboBox(videoGroupBox);
    videoCodecComboBox->addItems({"auto", "libx264", "libx265", "libvpx", "libvpx-vp9", "mpeg4", "copy"});
    videoCodecComboBox->setCurrentText("auto");
    connect(videoCodecComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TranscodePage::OnVideoCodecChanged);

    // Video Bitrate
    videoBitrateLabel = new QLabel("Bitrate:", videoGroupBox);
    videoBitrateSpinBox = new QSpinBox(videoGroupBox);
    videoBitrateSpinBox->setRange(0, 50000);
    videoBitrateSpinBox->setValue(0);
    videoBitrateSpinBox->setSuffix(" kbps");
    videoBitrateSpinBox->setSpecialValueText("auto");

    // Dimension
    dimensionLabel = new QLabel("Dimension:", videoGroupBox);
    widthSpinBox = new QSpinBox(videoGroupBox);
    widthSpinBox->setRange(0, 7680);
    widthSpinBox->setValue(0);
    widthSpinBox->setSpecialValueText("auto");

    dimensionXLabel = new QLabel("x", videoGroupBox);

    heightSpinBox = new QSpinBox(videoGroupBox);
    heightSpinBox->setRange(0, 4320);
    heightSpinBox->setValue(0);
    heightSpinBox->setSpecialValueText("auto");

    // Pixel Format
    pixFmtLabel = new QLabel("Pixel Format:", videoGroupBox);
    pixFmtComboBox = new QComboBox(videoGroupBox);
    pixFmtComboBox->addItems({"auto", "yuv420p", "yuv422p", "yuv444p", "rgb24", "bgr24"});
    pixFmtComboBox->setCurrentText("auto");

    videoLayout->addWidget(videoCodecLabel, 0, 0);
    videoLayout->addWidget(videoCodecComboBox, 0, 1, 1, 3);
    videoLayout->addWidget(videoBitrateLabel, 1, 0);
    videoLayout->addWidget(videoBitrateSpinBox, 1, 1, 1, 3);
    videoLayout->addWidget(dimensionLabel, 2, 0);
    videoLayout->addWidget(widthSpinBox, 2, 1);
    videoLayout->addWidget(dimensionXLabel, 2, 2);
    videoLayout->addWidget(heightSpinBox, 2, 3);
    videoLayout->addWidget(pixFmtLabel, 3, 0);
    videoLayout->addWidget(pixFmtComboBox, 3, 1, 1, 3);

    mainLayout->addWidget(videoGroupBox);

    // Audio Settings Section
    audioGroupBox = new QGroupBox("Audio Settings", this);
    QGridLayout *audioLayout = new QGridLayout(audioGroupBox);
    audioLayout->setSpacing(10);

    // Audio Codec
    audioCodecLabel = new QLabel("Codec:", audioGroupBox);
    audioCodecComboBox = new QComboBox(audioGroupBox);
    audioCodecComboBox->addItems({"auto", "aac", "libmp3lame", "libvorbis", "libopus", "copy"});
    audioCodecComboBox->setCurrentText("auto");

    // Audio Bitrate
    audioBitrateLabel = new QLabel("Bitrate:", audioGroupBox);
    audioBitrateSpinBox = new QSpinBox(audioGroupBox);
    audioBitrateSpinBox->setRange(0, 320);
    audioBitrateSpinBox->setValue(0);
    audioBitrateSpinBox->setSuffix(" kbps");
    audioBitrateSpinBox->setSpecialValueText("auto");

    audioLayout->addWidget(audioCodecLabel, 0, 0);
    audioLayout->addWidget(audioCodecComboBox, 0, 1);
    audioLayout->addWidget(audioBitrateLabel, 1, 0);
    audioLayout->addWidget(audioBitrateSpinBox, 1, 1);

    mainLayout->addWidget(audioGroupBox);

    // Preset Section
    presetGroupBox = new QGroupBox("Preset", this);
    QHBoxLayout *presetLayout = new QHBoxLayout(presetGroupBox);

    presetLabel = new QLabel("Preset:", presetGroupBox);
    presetComboBox = new QComboBox(presetGroupBox);
    presetComboBox->addItems({"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"});
    presetComboBox->setCurrentText("medium");

    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(presetComboBox);
    presetLayout->addStretch();

    mainLayout->addWidget(presetGroupBox);

    // Format Section
    formatGroupBox = new QGroupBox("File Format", this);
    QHBoxLayout *formatLayout = new QHBoxLayout(formatGroupBox);

    formatLabel = new QLabel("Format:", formatGroupBox);
    formatComboBox = new QComboBox(formatGroupBox);
    formatComboBox->addItems({"mp4", "mkv", "avi", "mov", "flv", "webm", "ts"});
    formatComboBox->setCurrentText("mp4");
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TranscodePage::OnFormatChanged);

    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(formatComboBox);
    formatLayout->addStretch();

    mainLayout->addWidget(formatGroupBox);

    // Progress Section
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);

    progressLabel = new QLabel("", this);
    progressLabel->setVisible(false);

    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(progressLabel);

    // Output File Section
    outputGroupBox = new QGroupBox("Output File", this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroupBox);

    QHBoxLayout *outputPathLayout = new QHBoxLayout();
    outputFileLineEdit = new QLineEdit(outputGroupBox);
    outputFileLineEdit->setPlaceholderText("Output file path will be generated automatically...");
    outputFileLineEdit->setReadOnly(true);

    browseOutputButton = new QPushButton("Browse...", outputGroupBox);
    connect(browseOutputButton, &QPushButton::clicked, this, &TranscodePage::OnBrowseOutputClicked);

    outputPathLayout->addWidget(outputFileLineEdit);
    outputPathLayout->addWidget(browseOutputButton);

    transcodeButton = new QPushButton("Transcode", outputGroupBox);
    transcodeButton->setEnabled(false);
    transcodeButton->setMinimumHeight(40);
    connect(transcodeButton, &QPushButton::clicked, this, &TranscodePage::OnTranscodeClicked);

    outputLayout->addLayout(outputPathLayout);
    outputLayout->addWidget(transcodeButton);

    mainLayout->addWidget(outputGroupBox);

    mainLayout->addStretch();

    setLayout(mainLayout);
}

void TranscodePage::OnBrowseInputClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Media File",
        "",
        "Media Files (*.mp4 *.mkv *.avi *.mov *.flv *.wmv *.webm *.ts *.m4v);;All Files (*.*)"
    );

    if (!fileName.isEmpty()) {
        inputFileLineEdit->setText(fileName);

        // Update shared input file path
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            mainWindow->GetSharedData()->SetInputFilePath(fileName);
        }

        // Set default format to same as input file
        QString ext = GetFileExtension(fileName);
        if (!ext.isEmpty()) {
            int index = formatComboBox->findText(ext);
            if (index >= 0) {
                formatComboBox->setCurrentIndex(index);
            }
        }

        UpdateOutputPath();
    }
}

void TranscodePage::OnBrowseOutputClicked() {
    QString format = formatComboBox->currentText();
    QString filter = QString("Media Files (*.%1);;All Files (*.*)").arg(format);

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Transcoded File",
        outputFileLineEdit->text(),
        filter
    );

    if (!fileName.isEmpty()) {
        outputFileLineEdit->setText(fileName);

        // Mark output path as manually set
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            mainWindow->GetSharedData()->SetOutputFilePath(fileName);
        }
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
    QString inputPath = inputFileLineEdit->text();
    QString outputPath = outputFileLineEdit->text();

    if (inputPath.isEmpty() || outputPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select input and output files.");
        return;
    }

    // Create encode parameters
    EncodeParameter *encodeParam = new EncodeParameter();
    ProcessParameter *processParam = new ProcessParameter();

    // Register this page as observer for progress updates
    processParam->add_observer(this);

    // Video settings
    QString videoCodec = videoCodecComboBox->currentText();
    if (videoCodec != "auto") {
        encodeParam->set_video_codec_name(videoCodec.toStdString());
    }

    int videoBitrate = videoBitrateSpinBox->value();
    if (videoBitrate > 0) {
        encodeParam->set_video_bit_rate(videoBitrate * 1000);
    }

    int width = widthSpinBox->value();
    if (width > 0) {
        encodeParam->set_width(width);
    }

    int height = heightSpinBox->value();
    if (height > 0) {
        encodeParam->set_height(height);
    }

    QString pixFmt = pixFmtComboBox->currentText();
    if (pixFmt != "auto") {
        encodeParam->set_pixel_format(pixFmt.toStdString());
    }

    // Audio settings
    QString audioCodec = audioCodecComboBox->currentText();
    if (audioCodec != "auto") {
        encodeParam->set_audio_codec_name(audioCodec.toStdString());
    }

    int audioBitrate = audioBitrateSpinBox->value();
    if (audioBitrate > 0) {
        encodeParam->set_audio_bit_rate(audioBitrate * 1000);
    }

    // Preset
    QString preset = presetComboBox->currentText();
    if (!preset.isEmpty()) {
        encodeParam->set_preset(preset.toStdString());
    }

    // Show progress bar
    progressBar->setValue(0);
    progressBar->setVisible(true);
    progressLabel->setText("Starting transcoding...");
    progressLabel->setVisible(true);

    // Disable button
    transcodeButton->setEnabled(false);
    transcodeButton->setText("Transcoding...");

    // Run transcoding in a separate thread
    RunTranscodeInThread(inputPath, outputPath, encodeParam, processParam);
}

void TranscodePage::RunTranscodeInThread(const QString &inputPath, const QString &outputPath,
                                         EncodeParameter *encodeParam, ProcessParameter *processParam) {
    QThread *thread = QThread::create([this, inputPath, outputPath, encodeParam, processParam]() {
        // Create converter
        Converter *converter = new Converter(processParam, encodeParam);
        converter->set_transcoder("FFMPEG");

        // Perform transcoding
        bool success = converter->convert_format(inputPath.toStdString(), outputPath.toStdString());

        // Clean up converter
        delete converter;

        // Emit signal to notify completion
        emit TranscodeComplete(success);
    });

    // Clean up thread when it finishes
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, [processParam, encodeParam]() {
        delete processParam;
        delete encodeParam;
    });

    thread->start();
}

void TranscodePage::OnTranscodeFinished(bool success) {
    // Hide progress bar
    progressBar->setVisible(false);
    progressLabel->setVisible(false);

    // Re-enable button
    transcodeButton->setEnabled(true);
    transcodeButton->setText("Transcode");

    if (success) {
        QMessageBox::information(this, "Success", "File transcoded successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to transcode file.");
    }
}

void TranscodePage::on_process_update(double progress) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, progress]() {
        progressBar->setValue(static_cast<int>(progress));
    }, Qt::QueuedConnection);
}

void TranscodePage::on_time_update(double timeRequired) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, timeRequired]() {
        int minutes = static_cast<int>(timeRequired) / 60;
        int seconds = static_cast<int>(timeRequired) % 60;
        progressLabel->setText(QString("Estimated time remaining: %1:%2")
                               .arg(minutes)
                               .arg(seconds, 2, 10, QChar('0')));
    }, Qt::QueuedConnection);
}

void TranscodePage::UpdateOutputPath() {
    QString inputPath = inputFileLineEdit->text();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString format = formatComboBox->currentText();
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath(format);
            outputFileLineEdit->setText(outputPath);
            transcodeButton->setEnabled(true);
        }
    }
}

QString TranscodePage::GetFileExtension(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower();
}
