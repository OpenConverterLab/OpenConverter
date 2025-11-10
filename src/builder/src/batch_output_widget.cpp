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

#include "../include/batch_output_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

BatchOutputWidget::BatchOutputWidget(QWidget *parent)
    : QWidget(parent),
      outputSuffix("-oc-output"),
      keepOriginalName(false) {
    SetupUI();
}

BatchOutputWidget::~BatchOutputWidget() {
}

void BatchOutputWidget::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Group box
    groupBox = new QGroupBox(tr("Batch Output"), this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);
    groupLayout->setSpacing(10);

    // Use output directory checkbox
    useOutputDirCheckBox = new QCheckBox(tr("Use custom output directory"), this);
    groupLayout->addWidget(useOutputDirCheckBox);

    // Output directory row
    QHBoxLayout *outputDirLayout = new QHBoxLayout();
    outputDirLabel = new QLabel(tr("Output Directory:"), this);
    outputDirLineEdit = new QLineEdit(this);
    outputDirLineEdit->setPlaceholderText(tr("Same as input directory"));
    outputDirLineEdit->setEnabled(false);
    browseOutputDirButton = new QPushButton(tr("Browse..."), this);
    browseOutputDirButton->setEnabled(false);

    outputDirLayout->addWidget(outputDirLabel);
    outputDirLayout->addWidget(outputDirLineEdit);
    outputDirLayout->addWidget(browseOutputDirButton);

    groupLayout->addLayout(outputDirLayout);

    // Suffix row
    QHBoxLayout *suffixLayout = new QHBoxLayout();
    suffixLabel = new QLabel(tr("Output Suffix:"), this);
    suffixLineEdit = new QLineEdit(this);
    suffixLineEdit->setText("-oc-output");
    suffixLineEdit->setPlaceholderText(tr("e.g., -oc-output"));

    suffixLayout->addWidget(suffixLabel);
    suffixLayout->addWidget(suffixLineEdit);
    suffixLayout->addStretch();

    groupLayout->addLayout(suffixLayout);

    // Keep original name checkbox
    keepOriginalNameCheckBox = new QCheckBox(tr("Keep original filename (no suffix)"), this);
    keepOriginalNameCheckBox->setEnabled(false);
    groupLayout->addWidget(keepOriginalNameCheckBox);

    // Example label
    exampleLabel = new QLabel(this);
    exampleLabel->setWordWrap(true);
    exampleLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    UpdateExampleLabel();
    groupLayout->addWidget(exampleLabel);

    mainLayout->addWidget(groupBox);

    // Connect signals
    connect(useOutputDirCheckBox, &QCheckBox::toggled, this, &BatchOutputWidget::OnUseOutputDirToggled);
    connect(browseOutputDirButton, &QPushButton::clicked, this, &BatchOutputWidget::OnBrowseOutputDirClicked);
    connect(suffixLineEdit, &QLineEdit::textChanged, this, &BatchOutputWidget::OnSuffixChanged);
    connect(keepOriginalNameCheckBox, &QCheckBox::toggled, this, &BatchOutputWidget::OnKeepOriginalNameToggled);
}

QString BatchOutputWidget::GetOutputDirectory() const {
    return outputDirectory;
}

QString BatchOutputWidget::GetOutputSuffix() const {
    return outputSuffix;
}

bool BatchOutputWidget::IsKeepOriginalName() const {
    return keepOriginalName;
}

bool BatchOutputWidget::IsUseOutputDirectory() const {
    return useOutputDirCheckBox->isChecked();
}

void BatchOutputWidget::SetOutputDirectory(const QString &dir) {
    outputDirectory = dir;
    outputDirLineEdit->setText(dir);
}

void BatchOutputWidget::SetOutputSuffix(const QString &suffix) {
    outputSuffix = suffix;
    suffixLineEdit->setText(suffix);
}

void BatchOutputWidget::SetKeepOriginalName(bool keep) {
    keepOriginalName = keep;
    keepOriginalNameCheckBox->setChecked(keep);
}

QString BatchOutputWidget::GenerateOutputPath(const QString &inputPath, const QString &outputExtension) const {
    QFileInfo inputInfo(inputPath);
    QString baseName = inputInfo.completeBaseName();
    QString extension = outputExtension.isEmpty() ? inputInfo.suffix() : outputExtension;

    QString outputDir;
    if (IsUseOutputDirectory() && !outputDirectory.isEmpty()) {
        outputDir = outputDirectory;
    } else {
        outputDir = inputInfo.absolutePath();
    }

    QString fileName;
    if (IsKeepOriginalName() && IsUseOutputDirectory()) {
        fileName = baseName + "." + extension;
    } else {
        fileName = baseName + outputSuffix + "." + extension;
    }

    return QDir(outputDir).filePath(fileName);
}

void BatchOutputWidget::OnBrowseOutputDirClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Directory"),
        outputDirectory.isEmpty() ? QString() : outputDirectory,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        SetOutputDirectory(dir);
        UpdateKeepOriginalNameState();
        UpdateExampleLabel();
        emit OutputConfigChanged();
    }
}

void BatchOutputWidget::OnUseOutputDirToggled(bool checked) {
    outputDirLineEdit->setEnabled(checked);
    browseOutputDirButton->setEnabled(checked);
    UpdateKeepOriginalNameState();
    UpdateExampleLabel();
    emit OutputConfigChanged();
}

void BatchOutputWidget::OnSuffixChanged(const QString &text) {
    outputSuffix = text;
    UpdateExampleLabel();
    emit OutputConfigChanged();
}

void BatchOutputWidget::OnKeepOriginalNameToggled(bool checked) {
    keepOriginalName = checked;
    suffixLineEdit->setEnabled(!checked);
    suffixLabel->setEnabled(!checked);
    UpdateExampleLabel();
    emit OutputConfigChanged();
}

void BatchOutputWidget::UpdateKeepOriginalNameState() {
    // Only enable "keep original name" if using custom output directory
    bool canKeepOriginal = IsUseOutputDirectory() && !outputDirectory.isEmpty();
    keepOriginalNameCheckBox->setEnabled(canKeepOriginal);

    if (!canKeepOriginal) {
        keepOriginalNameCheckBox->setChecked(false);
        keepOriginalName = false;
    }
}

void BatchOutputWidget::UpdateExampleLabel() {
    QString example;

    if (IsUseOutputDirectory() && !outputDirectory.isEmpty()) {
        if (IsKeepOriginalName()) {
            example = tr("Example: %1/video.mp4").arg(outputDirectory);
        } else {
            example = tr("Example: %1/video%2.mp4").arg(outputDirectory, outputSuffix);
        }
    } else {
        example = tr("Example: /input/dir/video%1.mp4").arg(outputSuffix);
    }

    exampleLabel->setText(example);
}

void BatchOutputWidget::RetranslateUi() {
    groupBox->setTitle(tr("Batch Output"));
    useOutputDirCheckBox->setText(tr("Use custom output directory"));
    outputDirLabel->setText(tr("Output Directory:"));
    outputDirLineEdit->setPlaceholderText(tr("Same as input directory"));
    browseOutputDirButton->setText(tr("Browse..."));
    suffixLabel->setText(tr("Output Suffix:"));
    suffixLineEdit->setPlaceholderText(tr("e.g., -oc-output"));
    keepOriginalNameCheckBox->setText(tr("Keep original filename (no suffix)"));
    UpdateExampleLabel();
}
