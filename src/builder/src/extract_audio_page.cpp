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

ExtractAudioPage::ExtractAudioPage(QWidget *parent) : BasePage(parent), converterRunner(nullptr) {
    SetupUI();
}

ExtractAudioPage::~ExtractAudioPage() {
}

void ExtractAudioPage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileSelector->GetLineEdit(), outputFileSelector->GetLineEdit(),
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

    // Input File Selector
    inputFileSelector = new FileSelectorWidget(
        tr("Input File"),
        FileSelectorWidget::InputFile,
        tr("Select a video file..."),
        tr("Video Files (*.mp4 *.avi *.mkv *.mov *.flv *.wmv *.webm);;All Files (*.*)"),
        tr("Select Video File"),
        this
    );
    connect(inputFileSelector, &FileSelectorWidget::FileSelected, this, &ExtractAudioPage::OnInputFileSelected);
    mainLayout->addWidget(inputFileSelector);

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
    progressWidget = new ProgressWidget(this);
    mainLayout->addWidget(progressWidget);

    // Output File Selector
    outputFileSelector = new FileSelectorWidget(
        tr("Output File"),
        FileSelectorWidget::OutputFile,
        tr("Output file path will be generated automatically..."),
        tr("Audio Files (*.mp3 *.aac *.ac3 *.flac *.wav *.ogg);;All Files (*.*)"),
        tr("Save Audio File"),
        this
    );
    connect(outputFileSelector, &FileSelectorWidget::FileSelected, this, &ExtractAudioPage::OnOutputFileSelected);
    mainLayout->addWidget(outputFileSelector);

    // Extract Button
    extractButton = new QPushButton(tr("Extract Audio"), this);
    extractButton->setEnabled(false);
    extractButton->setMinimumHeight(40);
    connect(extractButton, &QPushButton::clicked, this, &ExtractAudioPage::OnExtractClicked);
    mainLayout->addWidget(extractButton);

    // Create conversion runner
    converterRunner = new ConverterRunner(
        progressWidget->GetProgressBar(), progressWidget->GetProgressLabel(), extractButton,
        tr("Extracting..."), tr("Extract Audio"),
        tr("Success"), tr("Audio extracted successfully!"),
        tr("Error"), tr("Failed to extract audio."),
        this
    );
    connect(converterRunner, &ConverterRunner::ConversionFinished, this, &ExtractAudioPage::OnExtractFinished);

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void ExtractAudioPage::OnInputFileSelected(const QString &filePath) {
    // Update shared input file path
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetInputFilePath(filePath);
    }
    UpdateOutputPath();
}

void ExtractAudioPage::OnOutputFileSelected(const QString &filePath) {
    // Mark output path as manually set
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetOutputFilePath(filePath);
    }
}

void ExtractAudioPage::OnFormatChanged(int index) {
    Q_UNUSED(index);
    UpdateOutputPath();
}

void ExtractAudioPage::OnExtractClicked() {
    QString inputPath = inputFileSelector->GetFilePath();
    QString outputPath = outputFileSelector->GetFilePath();

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
        outputFileSelector->SetFilePath(outputPath);

        // Default use copy mode (no re-encoding)
        encodeParam->set_audio_codec_name("");
    }
    // Note: For non-auto formats, codec is auto-selected by backend based on output file extension

    // Disable video (extract audio only)
    encodeParam->set_video_codec_name("");

    if (bitrate > 0) {
        encodeParam->set_audio_bit_rate(bitrate * 1000);  // Convert kbps to bps
    }

    // Run conversion using ConverterRunner
    converterRunner->RunConversion(inputPath, outputPath, encodeParam, processParam);
}

void ExtractAudioPage::OnExtractFinished(bool success) {
    Q_UNUSED(success);
    // ConverterRunner handles all UI updates and message boxes
    // This slot is kept for potential custom post-processing
}

void ExtractAudioPage::UpdateOutputPath() {
    QString inputPath = inputFileSelector->GetFilePath();
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
            outputFileSelector->SetFilePath(outputPath);
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



void ExtractAudioPage::RetranslateUi() {
    // Update all translatable strings
    inputFileSelector->setTitle(tr("Input File"));
    inputFileSelector->SetPlaceholder(tr("Select a video file..."));
    inputFileSelector->GetBrowseButton()->setText(tr("Browse..."));

    settingsGroupBox->setTitle(tr("Audio Settings"));
    formatLabel->setText(tr("Output Format:"));
    bitrateLabel->setText(tr("Bitrate (kbps):"));
    bitrateSpinBox->setSpecialValueText(tr("auto"));
    bitrateSpinBox->setSuffix(tr(" kbps"));

    outputFileSelector->setTitle(tr("Output File"));
    outputFileSelector->SetPlaceholder(tr("Output file path will be generated automatically..."));
    outputFileSelector->GetBrowseButton()->setText(tr("Browse..."));
    extractButton->setText(tr("Extract Audio"));
}
