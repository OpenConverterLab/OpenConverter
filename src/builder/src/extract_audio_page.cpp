/*
 * Copyright 2024 Jack Lau
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

#include "../include/extract_audio_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include "../../common/include/encode_parameter.h"
#include "../../common/include/info.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>

ExtractAudioPage::ExtractAudioPage(QWidget *parent) : BasePage(parent) {
    SetupUI();
    connect(this, &ExtractAudioPage::ExtractComplete, this, &ExtractAudioPage::OnExtractFinished);
}

ExtractAudioPage::~ExtractAudioPage() {
}

void ExtractAudioPage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileLineEdit, outputFileLineEdit,
                           formatComboBox->currentText());
}

void ExtractAudioPage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void ExtractAudioPage::OnPageDeactivated() {
    BasePage::OnPageDeactivated();
}

void ExtractAudioPage::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input File Section
    inputGroupBox = new QGroupBox(tr("Input File"), this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroupBox);

    inputFileLineEdit = new QLineEdit(inputGroupBox);
    inputFileLineEdit->setPlaceholderText(tr("Select a video file..."));
    inputFileLineEdit->setReadOnly(true);

    browseInputButton = new QPushButton(tr("Browse..."), inputGroupBox);
    connect(browseInputButton, &QPushButton::clicked, this, &ExtractAudioPage::OnBrowseInputClicked);

    inputLayout->addWidget(inputFileLineEdit);
    inputLayout->addWidget(browseInputButton);

    mainLayout->addWidget(inputGroupBox);

    // Settings Section
    settingsGroupBox = new QGroupBox(tr("Audio Settings"), this);
    QGridLayout *settingsLayout = new QGridLayout(settingsGroupBox);
    settingsLayout->setSpacing(10);

    // Output Format
    formatLabel = new QLabel(tr("Output Format:"), settingsGroupBox);
    formatComboBox = new QComboBox(settingsGroupBox);
    formatComboBox->addItems({"auto", "aac", "mp3", "wav", "flac", "ogg", "m4a"});
    formatComboBox->setCurrentIndex(0);  // Default to "auto"
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExtractAudioPage::OnFormatChanged);

    settingsLayout->addWidget(formatLabel, 0, 0);
    settingsLayout->addWidget(formatComboBox, 0, 1);

    // Bitrate
    bitrateLabel = new QLabel(tr("Bitrate (kbps):"), settingsGroupBox);
    bitrateSpinBox = new QSpinBox(settingsGroupBox);
    bitrateSpinBox->setRange(0, 320);
    bitrateSpinBox->setValue(0);
    bitrateSpinBox->setSpecialValueText(tr("auto"));
    bitrateSpinBox->setSuffix(tr(" kbps"));

    settingsLayout->addWidget(bitrateLabel, 1, 0);
    settingsLayout->addWidget(bitrateSpinBox, 1, 1);

    mainLayout->addWidget(settingsGroupBox);

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
    outputGroupBox = new QGroupBox(tr("Output File"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroupBox);

    QHBoxLayout *outputPathLayout = new QHBoxLayout();
    outputFileLineEdit = new QLineEdit(outputGroupBox);
    outputFileLineEdit->setPlaceholderText(tr("Output file path will be generated automatically..."));
    outputFileLineEdit->setReadOnly(true);

    browseOutputButton = new QPushButton(tr("Browse..."), outputGroupBox);
    connect(browseOutputButton, &QPushButton::clicked, this, &ExtractAudioPage::OnBrowseOutputClicked);

    outputPathLayout->addWidget(outputFileLineEdit);
    outputPathLayout->addWidget(browseOutputButton);

    extractButton = new QPushButton(tr("Extract Audio"), outputGroupBox);
    extractButton->setEnabled(false);
    extractButton->setMinimumHeight(40);
    connect(extractButton, &QPushButton::clicked, this, &ExtractAudioPage::OnExtractClicked);

    outputLayout->addLayout(outputPathLayout);
    outputLayout->addWidget(extractButton);

    mainLayout->addWidget(outputGroupBox);

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void ExtractAudioPage::OnBrowseInputClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Video File"),
        "",
        tr("Video Files (*.mp4 *.avi *.mkv *.mov *.flv *.wmv *.webm);;All Files (*.*)")
    );

    if (!fileName.isEmpty()) {
        inputFileLineEdit->setText(fileName);

        // Update shared input file path
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            mainWindow->GetSharedData()->SetInputFilePath(fileName);
        }
        UpdateOutputPath();
    }
}

void ExtractAudioPage::OnBrowseOutputClicked() {
    QString format = formatComboBox->currentText();
    if (format == "auto") {
        format = "mp3";  // Default to mp3 for file dialog
    }

    QString filter = QString(tr("Audio Files (*.%1);;All Files (*.*)")).arg(format);

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Extracted Audio"),
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

void ExtractAudioPage::OnFormatChanged(int index) {
    Q_UNUSED(index);
    UpdateOutputPath();
}

void ExtractAudioPage::OnExtractClicked() {
    QString inputPath = inputFileLineEdit->text();
    QString outputPath = outputFileLineEdit->text();

    if (inputPath.isEmpty() || outputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please select input and output files."));
        return;
    }

    // Create parameters
    EncodeParameter *encodeParam = new EncodeParameter();
    ProcessParameter *processParam = new ProcessParameter();

    // Get settings
    QString format = formatComboBox->currentText();
    int bitrate = bitrateSpinBox->value();

    // Determine actual format
    if (format == "auto") {
        // Detect from input file
        QString detectedCodec = DetectAudioCodecFromFile(inputPath);
        format = MapCodecToFormat(detectedCodec);

        // Update output path with detected format
        QFileInfo inputInfo(inputPath);
        QString baseName = inputInfo.completeBaseName();
        QString dirPath = inputInfo.absolutePath();
        outputPath = QString("%1/%2-oc-output.%3").arg(dirPath, baseName, format);
        outputFileLineEdit->setText(outputPath);

        // Default use copy mode (no re-encoding)
        encodeParam->set_audio_codec_name("");
    } else {
        // Map format to codec
        QString codec = MapFormatToCodec(format);
        if (!codec.isEmpty()) {
            encodeParam->set_audio_codec_name(codec.toStdString());
        }
    }

    // Register this page as observer for progress updates
    processParam->add_observer(this);

    // Disable video (extract audio only)
    encodeParam->set_video_codec_name("");

    if (bitrate > 0) {
        encodeParam->set_audio_bit_rate(bitrate * 1000);  // Convert kbps to bps
    }

    // Show progress bar
    progressBar->setValue(0);
    progressBar->setVisible(true);
    progressLabel->setText("Starting audio extraction...");
    progressLabel->setVisible(true);

    // Disable button
    extractButton->setEnabled(false);
    extractButton->setText("Extracting...");

    // Run extraction in a separate thread
    RunExtractInThread(inputPath, outputPath, encodeParam, processParam);
}

void ExtractAudioPage::RunExtractInThread(const QString &inputPath, const QString &outputPath,
                                          EncodeParameter *encodeParam, ProcessParameter *processParam) {
    QThread *thread = QThread::create([this, inputPath, outputPath, encodeParam, processParam]() {
        // Create converter
        Converter *converter = new Converter(processParam, encodeParam);
        converter->set_transcoder("FFMPEG");

        // Perform extraction
        bool success = converter->convert_format(inputPath.toStdString(), outputPath.toStdString());

        // Clean up converter
        delete converter;

        // Emit signal to notify completion
        emit ExtractComplete(success);
    });

    // Clean up thread when it finishes
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, [processParam, encodeParam]() {
        delete processParam;
        delete encodeParam;
    });

    thread->start();
}

void ExtractAudioPage::OnExtractFinished(bool success) {
    // Hide progress bar
    progressBar->setVisible(false);
    progressLabel->setVisible(false);

    // Re-enable button
    extractButton->setEnabled(true);
    extractButton->setText("Extract Audio");

    if (success) {
        QMessageBox::information(this, "Success", "Audio extracted successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to extract audio.");
    }
}

void ExtractAudioPage::on_process_update(double progress) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, progress]() {
        progressBar->setValue(static_cast<int>(progress));
    }, Qt::QueuedConnection);
}

void ExtractAudioPage::on_time_update(double timeRequired) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, timeRequired]() {
        int minutes = static_cast<int>(timeRequired) / 60;
        int seconds = static_cast<int>(timeRequired) % 60;
        progressLabel->setText(QString("Estimated time remaining: %1:%2")
                               .arg(minutes)
                               .arg(seconds, 2, 10, QChar('0')));
    }, Qt::QueuedConnection);
}

void ExtractAudioPage::UpdateOutputPath() {
    QString inputPath = inputFileLineEdit->text();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString format = formatComboBox->currentText();
            if (format == "auto") {
                // Detect audio codec from input file and map to format
                QString detectedCodec = DetectAudioCodecFromFile(inputPath);
                format = MapCodecToFormat(detectedCodec);
            }
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath(format);
            outputFileLineEdit->setText(outputPath);
            extractButton->setEnabled(true);
        }
    }
}

QString ExtractAudioPage::DetectAudioCodecFromFile(const QString &filePath) {
    Info info;
    QByteArray ba = filePath.toLocal8Bit();
    char *src = ba.data();

    info.send_info(src);
    QuickInfo *quickInfo = info.get_quick_info();

    if (quickInfo && quickInfo->audioIdx >= 0) {
        return QString::fromStdString(quickInfo->audioCodec);
    }

    return QString();  // Return empty string if detection fails
}

QString ExtractAudioPage::MapCodecToFormat(const QString &codec) {
    if (codec.isEmpty()) {
        return "aac";  // Default fallback
    }

    if (codec == "aac") {
        return "aac";
    } else if (codec == "mp3") {
        return "mp3";
    } else if (codec == "pcm_s16le" || codec.startsWith("pcm_")) {
        return "wav";
    } else if (codec == "flac") {
        return "flac";
    } else if (codec == "vorbis") {
        return "ogg";
    } else {
        return "aac";  // Default fallback
    }
}

QString ExtractAudioPage::MapFormatToCodec(const QString &format) {
    if (format == "mp3") {
        return "libmp3lame";
    } else if (format == "aac" || format == "m4a") {
        return "aac";
    } else if (format == "wav") {
        return "pcm_s16le";
    } else if (format == "flac") {
        return "flac";
    } else if (format == "ogg") {
        return "libvorbis";
    } else {
        return "aac";  // Default
    }
}

void ExtractAudioPage::RetranslateUi() {
    // Update all translatable strings
    inputGroupBox->setTitle(tr("Input File"));
    inputFileLineEdit->setPlaceholderText(tr("Select a video file..."));
    browseInputButton->setText(tr("Browse..."));

    settingsGroupBox->setTitle(tr("Audio Settings"));
    formatLabel->setText(tr("Output Format:"));
    bitrateLabel->setText(tr("Bitrate (kbps):"));
    bitrateSpinBox->setSpecialValueText(tr("auto"));
    bitrateSpinBox->setSuffix(tr(" kbps"));

    outputGroupBox->setTitle(tr("Output File"));
    outputFileLineEdit->setPlaceholderText(tr("Output file path will be generated automatically..."));
    browseOutputButton->setText(tr("Browse..."));
    extractButton->setText(tr("Extract Audio"));
}
