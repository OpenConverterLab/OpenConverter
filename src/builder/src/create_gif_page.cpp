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
#include "../include/transcoder_helper.h"
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
    HandleSharedDataUpdate(inputFileSelector->GetLineEdit(), outputFileSelector->GetLineEdit(), "gif");
}

void CreateGifPage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void CreateGifPage::SetupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input File Selector
    inputFileSelector = new FileSelectorWidget(
        tr("Input File"),
        FileSelectorWidget::InputFile,
        tr("Select a video or image sequence..."),
        tr("Video Files (*.mp4 *.avi *.mkv *.mov *.flv *.wmv *.webm);;All Files (*.*)"),
        tr("Select Video File"),
        this
    );
    connect(inputFileSelector, &FileSelectorWidget::FileSelected, this, &CreateGifPage::OnInputFileSelected);
    mainLayout->addWidget(inputFileSelector);

    // Settings Group
    settingsGroupBox = new QGroupBox(tr("GIF Settings"), this);
    QGridLayout *settingsLayout = new QGridLayout(settingsGroupBox);
    settingsLayout->setColumnStretch(1, 1);

    // Resolution (Width x Height)
    resolutionLabel = new QLabel(tr("Resolution:"), this);
    resolutionWidget = new ResolutionWidget(this);
    resolutionWidget->SetRange(0, 4096);
    settingsLayout->addWidget(resolutionLabel, 0, 0);
    settingsLayout->addWidget(resolutionWidget, 0, 1);

    // TODO: FPS (Frame Rate)
    // fpsLabel = new QLabel(tr("Frame Rate (FPS):"), this);
    // fpsSpinBox = new QSpinBox(this);
    // fpsSpinBox->setRange(1, 60);
    // fpsSpinBox->setValue(10);
    // fpsSpinBox->setSuffix(tr(" fps"));
    // settingsLayout->addWidget(fpsLabel, 2, 0);
    // settingsLayout->addWidget(fpsSpinBox, 2, 1);

    mainLayout->addWidget(settingsGroupBox);

    // Output File Selector (for single file mode)
    outputFileSelector = new FileSelectorWidget(
        tr("Output"),
        FileSelectorWidget::OutputFile,
        tr("Output file path will be generated automatically..."),
        tr("GIF Files (*.gif);;All Files (*.*)"),
        tr("Save GIF File"),
        this
    );
    connect(outputFileSelector, &FileSelectorWidget::FileSelected, this, &CreateGifPage::OnOutputFileSelected);
    mainLayout->addWidget(outputFileSelector);

    // Batch Output Widget (for batch mode, replaces output file selector)
    batchOutputWidget = new BatchOutputWidget(this);
    batchOutputWidget->setVisible(false);
    mainLayout->addWidget(batchOutputWidget);

    // Convert Button
    convertButton = new QPushButton(tr("Create GIF / Add to Queue"), this);
    convertButton->setEnabled(false);
    convertButton->setMinimumHeight(40);
    connect(convertButton, &QPushButton::clicked, this, &CreateGifPage::OnConvertClicked);
    mainLayout->addWidget(convertButton);

    // Create batch mode helper
    batchModeHelper = new BatchModeHelper(
        inputFileSelector, batchOutputWidget, convertButton,
        tr("Create GIF / Add to Queue"), tr("Add to Queue"), this
    );
    batchModeHelper->SetSingleOutputWidget(outputFileSelector);  // Hide output selector in batch mode
    batchModeHelper->SetEncodeParameterCreator([this]() {
        return CreateEncodeParameter();
    });

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    // TODO: Connect FPS spin box value change signal
    // connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreateGifPage::OnFpsChanged);

    setLayout(mainLayout);
}

void CreateGifPage::OnInputFileSelected(const QString &filePath) {
    // Update shared input file path
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetInputFilePath(filePath);
    }
    UpdateOutputPath();
}

void CreateGifPage::OnOutputFileSelected(const QString &filePath) {
    // Mark output path as manually set
    OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
    if (mainWindow && mainWindow->GetSharedData()) {
        mainWindow->GetSharedData()->SetOutputFilePath(filePath);
    }
}

EncodeParameter* CreateGifPage::CreateEncodeParameter() {
    EncodeParameter *param = new EncodeParameter();

    // Set encode parameters for GIF creation
    // Note: Codec is auto-selected by backend based on output file extension (.gif)

    // TODO: Set frame rate for GIF
    // param->set_frame_rate(fpsSpinBox->value());

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

void CreateGifPage::OnConvertClicked() {
    // Check if batch mode is active
    if (batchModeHelper->IsBatchMode()) {
        // Batch mode: Add to queue
        batchModeHelper->AddToQueue("gif");
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

void CreateGifPage::OnFpsChanged(int value) {
    Q_UNUSED(value);
    // FPS change doesn't affect output path, but could be used for other purposes
}

void CreateGifPage::UpdateOutputPath() {
    QString inputPath = inputFileSelector->GetFilePath();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath("gif");
            outputFileSelector->SetFilePath(outputPath);
            convertButton->setEnabled(true);
        }
    }
}

void CreateGifPage::RetranslateUi() {
    // Update all translatable strings
    inputFileSelector->setTitle(tr("Input File"));
    inputFileSelector->SetPlaceholder(tr("Select a video or image sequence..."));
    inputFileSelector->RetranslateUi();

    settingsGroupBox->setTitle(tr("GIF Settings"));
    resolutionLabel->setText(tr("Resolution:"));
    resolutionWidget->RetranslateUi();
    // TODO: Frame Rate (FPS)
    // fpsLabel->setText(tr("Frame Rate (FPS):"));
    // fpsSpinBox->setSuffix(tr(" fps"));

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
            convertButton->setText(tr("Create GIF / Add to Queue"));
        }
    } else {
        convertButton->setText(tr("Create GIF"));
    }
}
