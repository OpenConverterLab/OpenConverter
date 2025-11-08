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

#include "../include/create_gif_page.h"
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

CreateGifPage::CreateGifPage(QWidget *parent) : BasePage(parent) {
    encodeParameter = new EncodeParameter();
    processParameter = new ProcessParameter();
    converter = new Converter(processParameter, encodeParameter);

    SetupUI();
}

CreateGifPage::~CreateGifPage() {
    delete converter;
    delete encodeParameter;
    delete processParameter;
}

QString CreateGifPage::GetPageTitle() const {
    return "Create Gif";
}

void CreateGifPage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileLineEdit, outputFileLineEdit, "gif");
}

void CreateGifPage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void CreateGifPage::SetupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input Group
    inputGroupBox = new QGroupBox(tr("Input File"), this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroupBox);

    inputFileLineEdit = new QLineEdit(this);
    inputFileLineEdit->setPlaceholderText(tr("Select a video or image sequence..."));
    inputFileLineEdit->setReadOnly(true);

    browseInputButton = new QPushButton(tr("Browse..."), this);

    inputLayout->addWidget(inputFileLineEdit, 1);
    inputLayout->addWidget(browseInputButton);

    mainLayout->addWidget(inputGroupBox);

    // Settings Group
    settingsGroupBox = new QGroupBox(tr("GIF Settings"), this);
    QGridLayout *settingsLayout = new QGridLayout(settingsGroupBox);
    settingsLayout->setColumnStretch(1, 1);

    // Width
    widthLabel = new QLabel(tr("Width (0 = auto):"), this);
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setRange(0, 4096);
    widthSpinBox->setValue(0);
    widthSpinBox->setSuffix(tr(" px"));
    settingsLayout->addWidget(widthLabel, 0, 0);
    settingsLayout->addWidget(widthSpinBox, 0, 1);

    // Height
    heightLabel = new QLabel(tr("Height (0 = auto):"), this);
    heightSpinBox = new QSpinBox(this);
    heightSpinBox->setRange(0, 4096);
    heightSpinBox->setValue(0);
    heightSpinBox->setSuffix(tr(" px"));
    settingsLayout->addWidget(heightLabel, 1, 0);
    settingsLayout->addWidget(heightSpinBox, 1, 1);

    // TODO: FPS (Frame Rate)
    // fpsLabel = new QLabel(tr("Frame Rate (FPS):"), this);
    // fpsSpinBox = new QSpinBox(this);
    // fpsSpinBox->setRange(1, 60);
    // fpsSpinBox->setValue(10);
    // fpsSpinBox->setSuffix(tr(" fps"));
    // settingsLayout->addWidget(fpsLabel, 2, 0);
    // settingsLayout->addWidget(fpsSpinBox, 2, 1);

    mainLayout->addWidget(settingsGroupBox);

    // Output Group
    outputGroupBox = new QGroupBox(tr("Output"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroupBox);

    QHBoxLayout *outputPathLayout = new QHBoxLayout();
    outputFileLineEdit = new QLineEdit(this);
    outputFileLineEdit->setPlaceholderText(tr("Output file path will be generated automatically..."));

    browseOutputButton = new QPushButton(tr("Browse..."), this);

    outputPathLayout->addWidget(outputFileLineEdit, 1);
    outputPathLayout->addWidget(browseOutputButton);

    convertButton = new QPushButton(tr("Create GIF"), this);
    convertButton->setEnabled(false);
    convertButton->setMinimumHeight(40);

    outputLayout->addLayout(outputPathLayout);
    outputLayout->addWidget(convertButton);

    mainLayout->addWidget(outputGroupBox);

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    // Connect signals
    connect(browseInputButton, &QPushButton::clicked, this, &CreateGifPage::OnBrowseInputClicked);
    connect(browseOutputButton, &QPushButton::clicked, this, &CreateGifPage::OnBrowseOutputClicked);
    connect(convertButton, &QPushButton::clicked, this, &CreateGifPage::OnConvertClicked);
    connect(inputFileLineEdit, &QLineEdit::textChanged, this, &CreateGifPage::OnInputFileChanged);
    // TODO: Connect FPS spin box value change signal
    // connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreateGifPage::OnFpsChanged);

    setLayout(mainLayout);
}

void CreateGifPage::OnBrowseInputClicked() {
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

void CreateGifPage::OnBrowseOutputClicked() {
    QString filter = tr("GIF Files (*.gif);;All Files (*.*)");

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save GIF File"),
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

void CreateGifPage::OnConvertClicked() {
    QString inputPath = inputFileLineEdit->text();
    QString outputPath = outputFileLineEdit->text();

    if (inputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an input file."));
        return;
    }

    if (outputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please specify an output file."));
        return;
    }

    // Set encode parameters for GIF creation
    // Note: Codec is auto-selected by backend based on output file extension (.gif)

    // TODO: Set frame rate for GIF
    // encodeParameter->set_frame_rate(fpsSpinBox->value());

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
    convertButton->setText(tr("Creating GIF..."));

    bool result = converter->convert_format(inputPath.toStdString(), outputPath.toStdString());

    convertButton->setEnabled(true);
    convertButton->setText(tr("Create GIF"));

    if (result) {
        QMessageBox::information(this, "Success", "GIF created successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to create GIF.");
    }
}

void CreateGifPage::OnInputFileChanged(const QString &text) {
    convertButton->setEnabled(!text.isEmpty() && !outputFileLineEdit->text().isEmpty());
}

void CreateGifPage::OnFpsChanged(int value) {
    Q_UNUSED(value);
    // FPS change doesn't affect output path, but could be used for other purposes
}

void CreateGifPage::UpdateOutputPath() {
    QString inputPath = inputFileLineEdit->text();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath("gif");
            outputFileLineEdit->setText(outputPath);
            convertButton->setEnabled(true);
        }
    }
}

void CreateGifPage::RetranslateUi() {
    // Update all translatable strings
    inputGroupBox->setTitle(tr("Input File"));
    inputFileLineEdit->setPlaceholderText(tr("Select a video or image sequence..."));
    browseInputButton->setText(tr("Browse..."));

    settingsGroupBox->setTitle(tr("GIF Settings"));
    widthLabel->setText(tr("Width (0 = auto):"));
    widthSpinBox->setSuffix(tr(" px"));
    heightLabel->setText(tr("Height (0 = auto):"));
    heightSpinBox->setSuffix(tr(" px"));
    // TODO: Frame Rate (FPS)
    // fpsLabel->setText(tr("Frame Rate (FPS):"));
    // fpsSpinBox->setSuffix(tr(" fps"));

    outputGroupBox->setTitle(tr("Output"));
    outputFileLineEdit->setPlaceholderText(tr("Output file path will be generated automatically..."));
    browseOutputButton->setText(tr("Browse..."));
    convertButton->setText(tr("Create GIF"));
}
