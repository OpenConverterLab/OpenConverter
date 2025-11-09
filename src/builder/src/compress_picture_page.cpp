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
                           formatComboBox->currentText());
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
    formatComboBox = new QComboBox(this);
    formatComboBox->addItems({"jpg", "png", "webp", "bmp", "tiff"});
    settingsLayout->addWidget(formatLabel, 0, 0);
    settingsLayout->addWidget(formatComboBox, 0, 1);

    // Width
    widthLabel = new QLabel(tr("Width (0 = auto):"), this);
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setRange(0, 16384);
    widthSpinBox->setValue(0);
    widthSpinBox->setSuffix(tr(" px"));
    settingsLayout->addWidget(widthLabel, 1, 0);
    settingsLayout->addWidget(widthSpinBox, 1, 1);

    // Height
    heightLabel = new QLabel(tr("Height (0 = auto):"), this);
    heightSpinBox = new QSpinBox(this);
    heightSpinBox->setRange(0, 16384);
    heightSpinBox->setValue(0);
    heightSpinBox->setSuffix(tr(" px"));
    settingsLayout->addWidget(heightLabel, 2, 0);
    settingsLayout->addWidget(heightSpinBox, 2, 1);

    // Pixel Format
    pixFmtLabel = new QLabel(tr("Pixel Format:"), this);
    pixFmtComboBox = new QComboBox(this);
    pixFmtComboBox->addItems({
        "auto",
        // RGB formats
        "rgb24", "rgba", "rgb48be", "rgba64be",
        // YUV formats (limited range)
        "yuv420p", "yuv422p", "yuv444p",
        // YUVJ formats (full range, common for JPEG)
        "yuvj420p", "yuvj422p", "yuvj444p",
        // Grayscale
        "gray", "gray16be",
        // Other common formats
        "bgr24", "bgra"
    });
    settingsLayout->addWidget(pixFmtLabel, 3, 0);
    settingsLayout->addWidget(pixFmtComboBox, 3, 1);

    // Quality (qscale)
    qualityLabel = new QLabel(tr("Quality (2-31, lower=better):"), this);
    qualitySpinBox = new QSpinBox(this);
    qualitySpinBox->setRange(2, 31);
    qualitySpinBox->setValue(5);
    settingsLayout->addWidget(qualityLabel, 4, 0);
    settingsLayout->addWidget(qualitySpinBox, 4, 1);

    mainLayout->addWidget(settingsGroupBox);

    // Output File Selector
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

    // Convert Button
    convertButton = new QPushButton(tr("Convert"), this);
    convertButton->setEnabled(false);
    convertButton->setMinimumHeight(40);
    connect(convertButton, &QPushButton::clicked, this, &CompressPicturePage::OnConvertClicked);
    mainLayout->addWidget(convertButton);

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    // Connect format change signal
    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CompressPicturePage::OnFormatChanged);

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

void CompressPicturePage::OnConvertClicked() {
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

    // Set encode parameters
    // Note: Codec is auto-selected by backend based on output file extension
    encodeParameter->set_qscale(qualitySpinBox->value());
    if (pixFmtComboBox->currentText() != "auto")
        encodeParameter->set_pixel_format(pixFmtComboBox->currentText().toStdString());

    if (widthSpinBox->value() > 0) {
        encodeParameter->set_width(widthSpinBox->value());
    }

    if (heightSpinBox->value() > 0) {
        encodeParameter->set_height(heightSpinBox->value());
    }

    // Only support FFmpeg transcoder
    if (!converter->set_transcoder("FFMPEG")) {
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

void CompressPicturePage::OnFormatChanged(int index) {
    Q_UNUSED(index);
    UpdateOutputPath();
}

void CompressPicturePage::UpdateOutputPath() {
    QString inputPath = inputFileSelector->GetFilePath();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString format = formatComboBox->currentText();
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
    inputFileSelector->GetBrowseButton()->setText(tr("Browse..."));

    settingsGroupBox->setTitle(tr("Compression Settings"));
    formatLabel->setText(tr("Output Format:"));
    widthLabel->setText(tr("Width (0 = auto):"));
    widthSpinBox->setSuffix(tr(" px"));
    heightLabel->setText(tr("Height (0 = auto):"));
    heightSpinBox->setSuffix(tr(" px"));
    pixFmtLabel->setText(tr("Pixel Format:"));
    qualityLabel->setText(tr("Quality (2-31, lower=better):"));

    outputFileSelector->setTitle(tr("Output"));
    outputFileSelector->SetPlaceholder(tr("Output file path will be generated automatically..."));
    outputFileSelector->GetBrowseButton()->setText(tr("Browse..."));
    convertButton->setText(tr("Convert"));
}
