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

#include "../include/ai_processing_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include "../include/batch_queue.h"
#include "../include/batch_item.h"
#include "../include/transcoder_helper.h"
#include "../include/python_manager.h"
#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include "../../component/include/python_install_dialog.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>

AIProcessingPage::AIProcessingPage(QWidget *parent) : BasePage(parent), converterRunner(nullptr) {
    SetupUI();
}

AIProcessingPage::~AIProcessingPage() {
}

void AIProcessingPage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileSelector->GetLineEdit(), outputFileSelector->GetLineEdit(),
                           GetFileExtension(inputFileSelector->GetFilePath()));

    // Check if Python is installed for AI Processing
    // In Debug mode, skip installation dialog (assume developer has configured environment)
#ifdef NDEBUG
    // Release mode: check Python and offer installation
    PythonManager pythonManager;

    // Check status: embedded Python, system Python, or not installed
    if (pythonManager.GetStatus() != PythonManager::Status::Installed) {
        // Python not available (neither embedded nor system)
        // Show installation dialog
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Python Required"),
            tr("AI Processing requires Python 3.9 and additional packages.\n\n"
               "Would you like to download and install them now?\n"
               "(Download size: ~550 MB, completely isolated from system Python)"),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            PythonInstallDialog dialog(this);
            if (dialog.exec() != QDialog::Accepted) {
                // User cancelled installation
                QMessageBox::information(
                    this,
                    tr("AI Processing Unavailable"),
                    tr("AI Processing features require Python to be installed.\n\n"
                       "You can install it later by returning to this page.")
                );
            }
        }
    }
#else
    // Debug mode: assume developer has configured Python environment
    // Skip installation dialog
    qDebug() << "Debug mode: Skipping Python installation check (assuming developer environment)";
#endif
}

void AIProcessingPage::OnInputFileChanged(const QString &newPath) {
    QString ext = GetFileExtension(newPath);
    if (!ext.isEmpty()) {
        int index = formatComboBox->findText(ext);
        if (index >= 0) {
            formatComboBox->setCurrentIndex(index);
        }
    }
    // Update output path when input changes
    UpdateOutputPath();
}

void AIProcessingPage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void AIProcessingPage::OnPageDeactivated() {
    BasePage::OnPageDeactivated();
    HandleSharedDataUpdate(inputFileSelector->GetLineEdit(), outputFileSelector->GetLineEdit(),
                           formatComboBox->currentText());
}

void AIProcessingPage::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input File Selector (with Batch button)
    inputFileSelector = new FileSelectorWidget(
        tr("Input File"),
        FileSelectorWidget::InputFile,
        tr("Select a media file or click Batch for multiple files..."),
        tr("All Files (*.*)"),
        tr("Select Media File"),
        this
    );
    connect(inputFileSelector, &FileSelectorWidget::FileSelected, this, &AIProcessingPage::OnInputFileSelected);
    mainLayout->addWidget(inputFileSelector);

    // Algorithm Selection Section
    algorithmGroupBox = new QGroupBox(tr("Algorithm"), this);
    QGridLayout *algorithmLayout = new QGridLayout(algorithmGroupBox);
    algorithmLayout->setSpacing(10);

    algorithmLabel = new QLabel(tr("Select Algorithm:"), algorithmGroupBox);
    algorithmComboBox = new QComboBox(algorithmGroupBox);
    algorithmComboBox->addItem(tr("Upscaler"));
    algorithmComboBox->setCurrentIndex(0);
    connect(algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AIProcessingPage::OnAlgorithmChanged);

    algorithmLayout->addWidget(algorithmLabel, 0, 0);
    algorithmLayout->addWidget(algorithmComboBox, 0, 1);

    mainLayout->addWidget(algorithmGroupBox);

    // Algorithm Settings Section (dynamic based on selected algorithm)
    algoSettingsGroupBox = new QGroupBox(tr("Algorithm Settings"), this);
    QGridLayout *algoSettingsLayout = new QGridLayout(algoSettingsGroupBox);
    algoSettingsLayout->setSpacing(10);

    algoSettingsStack = new QStackedWidget(algoSettingsGroupBox);
    algoSettingsStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Upscaler Settings Widget
    upscalerSettingsWidget = new QWidget(algoSettingsStack);
    QGridLayout *upscalerLayout = new QGridLayout(upscalerSettingsWidget);
    upscalerLayout->setSpacing(10);
    upscalerLayout->setContentsMargins(0, 0, 0, 0);

    upscaleFactorLabel = new QLabel(tr("Upscale Factor:"), upscalerSettingsWidget);
    upscaleFactorSpinBox = new QSpinBox(upscalerSettingsWidget);
    upscaleFactorSpinBox->setMinimum(2);
    upscaleFactorSpinBox->setMaximum(8);
    upscaleFactorSpinBox->setValue(2);
    upscaleFactorSpinBox->setSuffix("x");

    upscalerLayout->addWidget(upscaleFactorLabel, 0, 0);
    upscalerLayout->addWidget(upscaleFactorSpinBox, 0, 1);

    algoSettingsStack->addWidget(upscalerSettingsWidget);

    algoSettingsLayout->addWidget(algoSettingsStack, 0, 0, 1, 2);
    mainLayout->addWidget(algoSettingsGroupBox);

    // Video Settings Section
    videoGroupBox = new QGroupBox(tr("Video Settings"), this);
    QGridLayout *videoLayout = new QGridLayout(videoGroupBox);
    videoLayout->setSpacing(10);

    videoCodecLabel = new QLabel(tr("Codec:"), videoGroupBox);
    videoCodecComboBox = new QComboBox(videoGroupBox);
    videoCodecComboBox->addItems({"auto", "libx264", "libx265", "libvpx-vp9", "copy"});
    videoCodecComboBox->setCurrentText("auto");

    videoBitrateLabel = new QLabel(tr("Bitrate:"), videoGroupBox);
    videoBitrateWidget = new BitrateWidget(BitrateWidget::Video, videoGroupBox);

    videoLayout->addWidget(videoCodecLabel, 0, 0);
    videoLayout->addWidget(videoCodecComboBox, 0, 1);
    videoLayout->addWidget(videoBitrateLabel, 1, 0);
    videoLayout->addWidget(videoBitrateWidget, 1, 1);

    mainLayout->addWidget(videoGroupBox);

    // Audio Settings Section
    audioGroupBox = new QGroupBox(tr("Audio Settings"), this);
    QGridLayout *audioLayout = new QGridLayout(audioGroupBox);
    audioLayout->setSpacing(10);

    audioCodecLabel = new QLabel(tr("Codec:"), audioGroupBox);
    audioCodecComboBox = new QComboBox(audioGroupBox);
    audioCodecComboBox->addItems({"auto", "aac", "libmp3lame", "libopus", "copy"});
    audioCodecComboBox->setCurrentText("auto");

    audioBitrateLabel = new QLabel(tr("Bitrate:"), audioGroupBox);
    audioBitrateWidget = new BitrateWidget(BitrateWidget::Audio, audioGroupBox);

    audioLayout->addWidget(audioCodecLabel, 0, 0);
    audioLayout->addWidget(audioCodecComboBox, 0, 1);
    audioLayout->addWidget(audioBitrateLabel, 1, 0);
    audioLayout->addWidget(audioBitrateWidget, 1, 1);

    mainLayout->addWidget(audioGroupBox);

    // Format Section
    formatGroupBox = new QGroupBox(tr("File Format"), this);
    QHBoxLayout *formatLayout = new QHBoxLayout(formatGroupBox);

    formatLabel = new QLabel(tr("Format:"), formatGroupBox);
    formatComboBox = new QComboBox(formatGroupBox);
    formatComboBox->addItems({"mp4", "mkv", "avi", "mov", "flv", "webm", "ts", "jpg", "png"});
    formatComboBox->setCurrentText("mp4");
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AIProcessingPage::OnFormatChanged);

    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(formatComboBox);
    formatLayout->addStretch();

    mainLayout->addWidget(formatGroupBox);

    // Output File Selector
    outputFileSelector = new FileSelectorWidget(
        tr("Output File"),
        FileSelectorWidget::OutputFile,
        tr("Output file path..."),
        tr("All Files (*.*)"),
        tr("Select Output File"),
        this
    );
    connect(outputFileSelector, &FileSelectorWidget::FileSelected, this, &AIProcessingPage::OnOutputFileSelected);
    mainLayout->addWidget(outputFileSelector);

    // Batch Output Widget (hidden by default, shown when batch files selected)
    batchOutputWidget = new BatchOutputWidget(this);
    batchOutputWidget->setVisible(false);
    mainLayout->addWidget(batchOutputWidget);

    // Process Button
    processButton = new QPushButton(tr("Process / Add to Queue"), this);
    processButton->setEnabled(false);
    processButton->setMinimumHeight(40);
    connect(processButton, &QPushButton::clicked, this, &AIProcessingPage::OnProcessClicked);
    mainLayout->addWidget(processButton);

    // Progress Section (placed after button to avoid blank space when hidden)
    progressWidget = new ProgressWidget(this);
    mainLayout->addWidget(progressWidget);

    // Initialize converter runner
    converterRunner = new ConverterRunner(
        progressWidget->GetProgressBar(), progressWidget->GetProgressLabel(), processButton,
        tr("Processing..."), tr("Process / Add to Queue"),
        tr("Success"), tr("AI processing completed successfully!"),
        tr("Error"), tr("Failed to process file."),
        this
    );
    connect(converterRunner, &ConverterRunner::ConversionFinished, this, &AIProcessingPage::OnProcessFinished);

    // Initialize batch mode helper
    batchModeHelper = new BatchModeHelper(
        inputFileSelector, batchOutputWidget, processButton,
        tr("Process / Add to Queue"), tr("Add to Queue"), this
    );
    batchModeHelper->SetSingleOutputWidget(outputFileSelector);
    batchModeHelper->SetEncodeParameterCreator([this]() {
        return CreateEncodeParameter();
    });
}

void AIProcessingPage::OnInputFileSelected(const QString &filePath) {
    if (filePath.isEmpty()) {
        return;
    }

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

    // Update output path
    UpdateOutputPath();
}

void AIProcessingPage::OnOutputFileSelected(const QString &filePath) {
    // Mark output path as manually set
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetOutputFilePath(filePath);
    }
}

void AIProcessingPage::OnAlgorithmChanged(int index) {
    // Switch to the corresponding settings widget
    algoSettingsStack->setCurrentIndex(index);
}

void AIProcessingPage::OnFormatChanged(int index) {
    Q_UNUSED(index);
    UpdateOutputPath();
}

void AIProcessingPage::OnProcessClicked() {
    // Check if batch mode is active
    if (batchModeHelper->IsBatchMode()) {
        // Batch mode: Add to queue
        QString format = formatComboBox->currentText();
        batchModeHelper->AddToQueue(format);
        return;
    }

    // Single file mode: Process immediately
    QString inputPath = inputFileSelector->GetFilePath();
    QString outputPath = outputFileSelector->GetFilePath();

    if (inputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an input file."));
        return;
    }

    if (outputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an output file."));
        return;
    }

    // Create parameters
    EncodeParameter *encodeParam = CreateEncodeParameter();
    ProcessParameter *processParam = new ProcessParameter();

    // Get transcoder name (must be BMF for AI processing)
    QString transcoderName = "BMF";

    // Run conversion using ConverterRunner
    converterRunner->RunConversion(inputPath, outputPath, encodeParam, processParam, transcoderName);
}

void AIProcessingPage::OnProcessFinished(bool success) {
    Q_UNUSED(success);
    // ConverterRunner handles all UI updates and message boxes
    // This slot is kept for potential custom post-processing
    emit ProcessComplete(success);
}

void AIProcessingPage::UpdateOutputPath() {
    QString inputPath = inputFileSelector->GetFilePath();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString format = formatComboBox->currentText();
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath(format);
            outputFileSelector->SetFilePath(outputPath);
            processButton->setEnabled(true);
        }
    }
}

QString AIProcessingPage::GetFileExtension(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix();
}

EncodeParameter* AIProcessingPage::CreateEncodeParameter() {
    EncodeParameter *encodeParam = new EncodeParameter();

    // Set algorithm mode based on selected algorithm
    int algoIndex = algorithmComboBox->currentIndex();
    if (algoIndex == 0) {  // Upscaler
        encodeParam->set_algo_mode(AlgoMode::Upscale);
        encodeParam->set_upscale_factor(upscaleFactorSpinBox->value());
    }

    // Set video codec and bitrate
    QString videoCodec = videoCodecComboBox->currentText();
    encodeParam->set_video_codec_name(videoCodec.toStdString());

    int videoBitrate = videoBitrateWidget->GetBitrate();
    if (videoBitrate > 0) {
        encodeParam->set_video_bit_rate(videoBitrate);
    }

    // Set audio codec and bitrate
    QString audioCodec = audioCodecComboBox->currentText();
    encodeParam->set_audio_codec_name(audioCodec.toStdString());

    int audioBitrate = audioBitrateWidget->GetBitrate();
    if (audioBitrate > 0) {
        encodeParam->set_audio_bit_rate(audioBitrate);
    }

    return encodeParam;
}

void AIProcessingPage::RetranslateUi() {
    // Update all translatable strings
    algorithmGroupBox->setTitle(tr("Algorithm"));
    algorithmLabel->setText(tr("Select Algorithm:"));
    algorithmComboBox->setItemText(0, tr("Upscaler"));

    algoSettingsGroupBox->setTitle(tr("Algorithm Settings"));
    upscaleFactorLabel->setText(tr("Upscale Factor:"));

    videoGroupBox->setTitle(tr("Video Settings"));
    videoCodecLabel->setText(tr("Codec:"));
    videoBitrateLabel->setText(tr("Bitrate:"));

    audioGroupBox->setTitle(tr("Audio Settings"));
    audioCodecLabel->setText(tr("Codec:"));
    audioBitrateLabel->setText(tr("Bitrate:"));

    // Update button text based on batch mode
    if (batchModeHelper) {
        if (inputFileSelector->IsBatchMode()) {
            processButton->setText(tr("Add to Queue"));
        } else {
            processButton->setText(tr("Process / Add to Queue"));
        }
    }
}
