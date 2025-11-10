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

#include "../include/compress_picture_page.h"
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

CompressPicturePage::CompressPicturePage(QWidget *parent) : BasePage(parent) {
    encodeParameter = new EncodeParameter();
    processParameter = new ProcessParameter();
    converter = new Converter(processParameter, encodeParameter);

    SetupUI();
}

CompressPicturePage::~CompressPicturePage() {
    delete converter;
    delete encodeParameter;
    delete processParameter;
}

QString CompressPicturePage::GetPageTitle() const {
    return "Compress Picture";
}

void CompressPicturePage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileSelector->GetLineEdit(), outputFileSelector->GetLineEdit(),
                           formatWidget->GetFormat());
}

void CompressPicturePage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void CompressPicturePage::SetupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input File Selector
    inputFileSelector = new FileSelectorWidget(
        tr("Input File"),
        FileSelectorWidget::InputFile,
        tr("Select an image file..."),
        tr("Image Files (*.jpg *.jpeg *.png *.bmp *.tiff *.webp *.gif);;All Files (*.*)"),
        tr("Select Image File"),
        this
    );
    connect(inputFileSelector, &FileSelectorWidget::FileSelected, this, &CompressPicturePage::OnInputFileSelected);
    mainLayout->addWidget(inputFileSelector);

    // Settings Group
    settingsGroupBox = new QGroupBox(tr("Compression Settings"), this);
    QGridLayout *settingsLayout = new QGridLayout(settingsGroupBox);
    settingsLayout->setColumnStretch(1, 1);

    // Format
    formatLabel = new QLabel(tr("Output Format:"), this);
    formatWidget = new FormatSelectorWidget(FormatSelectorWidget::Image, false, this);
    settingsLayout->addWidget(formatLabel, 0, 0);
    settingsLayout->addWidget(formatWidget, 0, 1);

    // Resolution (Width x Height)
    resolutionLabel = new QLabel(tr("Resolution:"), this);
    resolutionWidget = new ResolutionWidget(this);
    settingsLayout->addWidget(resolutionLabel, 1, 0);
    settingsLayout->addWidget(resolutionWidget, 1, 1);

    // Pixel Format
    pixelFormatLabel = new QLabel(tr("Pixel Format:"), this);
    pixelFormatWidget = new PixelFormatWidget(PixelFormatWidget::Image, this);
    settingsLayout->addWidget(pixelFormatLabel, 2, 0);
    settingsLayout->addWidget(pixelFormatWidget, 2, 1);

    // Quality (qscale)
    qualityLabel = new QLabel(tr("Quality:"), this);
    qualityWidget = new QualityWidget(QualityWidget::Image, this);
    settingsLayout->addWidget(qualityLabel, 3, 0);
    settingsLayout->addWidget(qualityWidget, 3, 1);

    mainLayout->addWidget(settingsGroupBox);

    // Output File Selector (for single file mode)
    outputFileSelector = new FileSelectorWidget(
        tr("Output"),
        FileSelectorWidget::OutputFile,
        tr("Output file path will be generated automatically..."),
        tr("Image Files (*.jpg *.png *.webp *.bmp *.tiff);;All Files (*.*)"),
        tr("Save Image File"),
        this
    );
    connect(outputFileSelector, &FileSelectorWidget::FileSelected, this, &CompressPicturePage::OnOutputFileSelected);
    mainLayout->addWidget(outputFileSelector);

    // Batch Output Widget (for batch mode, replaces output file selector)
    batchOutputWidget = new BatchOutputWidget(this);
    batchOutputWidget->setVisible(false);
    mainLayout->addWidget(batchOutputWidget);

    // Convert Button
    convertButton = new QPushButton(tr("Convert / Add to Queue"), this);
    convertButton->setEnabled(false);
    convertButton->setMinimumHeight(40);
    connect(convertButton, &QPushButton::clicked, this, &CompressPicturePage::OnConvertClicked);
    mainLayout->addWidget(convertButton);

    // Create batch mode helper
    batchModeHelper = new BatchModeHelper(
        inputFileSelector, batchOutputWidget, convertButton,
        tr("Convert / Add to Queue"), tr("Add to Queue"), this
    );
    batchModeHelper->SetSingleOutputWidget(outputFileSelector);  // Hide output selector in batch mode
    batchModeHelper->SetEncodeParameterCreator([this]() {
        return CreateEncodeParameter();
    });

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    // Connect format change signal
    connect(formatWidget, &FormatSelectorWidget::FormatChanged, this,
            QOverload<const QString &>::of(&CompressPicturePage::OnFormatChanged));

    setLayout(mainLayout);
}

void CompressPicturePage::OnInputFileSelected(const QString &filePath) {
    // Update shared input file path
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetInputFilePath(filePath);
    }
    UpdateOutputPath();
}

void CompressPicturePage::OnOutputFileSelected(const QString &filePath) {
    // Mark output path as manually set
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetOutputFilePath(filePath);
    }
}

EncodeParameter* CompressPicturePage::CreateEncodeParameter() {
    EncodeParameter *param = new EncodeParameter();

    // Set encode parameters
    // Note: Codec is auto-selected by backend based on output file extension
    param->set_qscale(qualityWidget->GetQuality());

    // Pixel format
    QString pixFmt = pixelFormatWidget->GetPixelFormat();
    if (!pixFmt.isEmpty()) {
        param->set_pixel_format(pixFmt.toStdString());
    }

    // Resolution
    int width = resolutionWidget->GetWidth();
    int height = resolutionWidget->GetHeight();
    if (width > 0) {
        param->set_width(width);
    }
    if (height > 0) {
        param->set_height(height);
    }

    return param;
}

void CompressPicturePage::OnConvertClicked() {
    // Check if batch mode is active
    if (batchModeHelper->IsBatchMode()) {
        // Batch mode: Add to queue
        QString format = formatWidget->GetFormat();
        batchModeHelper->AddToQueue(format);
        return;
    }

    // Single file mode: Convert immediately
    QString inputPath = inputFileSelector->GetFilePath();
    QString outputPath = outputFileSelector->GetFilePath();

    if (inputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an input file."));
        return;
    }

    if (outputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please specify an output file."));
        return;
    }

    // Get encode parameters
    EncodeParameter *tempParam = CreateEncodeParameter();
    encodeParameter->set_qscale(tempParam->get_qscale());
    if (tempParam->get_pixel_format() != "")
        encodeParameter->set_pixel_format(tempParam->get_pixel_format());
    if (tempParam->get_width() > 0)
        encodeParameter->set_width(tempParam->get_width());
    if (tempParam->get_height() > 0)
        encodeParameter->set_height(tempParam->get_height());
    delete tempParam;

    // Get current transcoder from main window
    QString transcoderName = TranscoderHelper::GetCurrentTranscoderName(this);

    // Set transcoder
    if (!converter->set_transcoder(transcoderName.toStdString())) {
        QMessageBox::critical(this, "Error", "Failed to initialize transcoder.");
        return;
    }

    // Perform conversion
    convertButton->setEnabled(false);
    convertButton->setText(tr("Converting..."));

    bool result = converter->convert_format(inputPath.toStdString(), outputPath.toStdString());

    convertButton->setEnabled(true);
    convertButton->setText(tr("Convert"));

    if (result) {
        QMessageBox::information(this, "Success", "Image compressed successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to compress image.");
    }
}

void CompressPicturePage::OnFormatChanged(const QString &format) {
    Q_UNUSED(format);
    UpdateOutputPath();
}

void CompressPicturePage::UpdateOutputPath() {
    QString inputPath = inputFileSelector->GetFilePath();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString format = formatWidget->GetFormat();
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath(format);
            outputFileSelector->SetFilePath(outputPath);
            convertButton->setEnabled(true);
        }
    }
}

void CompressPicturePage::RetranslateUi() {
    // Update all translatable strings
    inputFileSelector->setTitle(tr("Input File"));
    inputFileSelector->SetPlaceholder(tr("Select an image file..."));
    inputFileSelector->RetranslateUi();

    settingsGroupBox->setTitle(tr("Compression Settings"));
    formatLabel->setText(tr("Output Format:"));
    formatWidget->RetranslateUi();
    resolutionLabel->setText(tr("Resolution:"));
    resolutionWidget->RetranslateUi();
    pixelFormatLabel->setText(tr("Pixel Format:"));
    pixelFormatWidget->RetranslateUi();
    qualityLabel->setText(tr("Quality:"));
    qualityWidget->RetranslateUi();

    outputFileSelector->setTitle(tr("Output"));
    outputFileSelector->SetPlaceholder(tr("Output file path will be generated automatically..."));
    outputFileSelector->RetranslateUi();

    // Update batch widgets
    batchOutputWidget->RetranslateUi();

    // Update button text based on batch mode
    if (batchModeHelper) {
        if (inputFileSelector->IsBatchMode()) {
            convertButton->setText(tr("Add to Queue"));
        } else {
            convertButton->setText(tr("Convert / Add to Queue"));
        }
    } else {
        convertButton->setText(tr("Convert"));
    }
}
