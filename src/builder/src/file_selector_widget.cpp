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

#include "../include/file_selector_widget.h"
#include <QFileDialog>
#include <QHBoxLayout>

FileSelectorWidget::FileSelectorWidget(const QString &title, SelectorType type, QWidget *parent)
    : QGroupBox(title, parent), selectorType(type) {

    // Default values
    fileFilter = "All Files (*.*)";
    dialogTitle = (type == InputFile) ? "Select File" : "Save File";

    SetupUI();
}

FileSelectorWidget::FileSelectorWidget(const QString &title,
                                       SelectorType type,
                                       const QString &placeholder,
                                       const QString &fileFilter,
                                       const QString &dialogTitle,
                                       QWidget *parent)
    : QGroupBox(title, parent), selectorType(type), fileFilter(fileFilter), dialogTitle(dialogTitle) {

    SetupUI();
    fileLineEdit->setPlaceholderText(placeholder);
}

void FileSelectorWidget::SetupUI() {
    QHBoxLayout *layout = new QHBoxLayout(this);

    fileLineEdit = new QLineEdit(this);
    fileLineEdit->setReadOnly(true);

    browseButton = new QPushButton(tr("Browse..."), this);
    connect(browseButton, &QPushButton::clicked, this, &FileSelectorWidget::OnBrowseClicked);

    layout->addWidget(fileLineEdit, 1);  // Stretch factor 1 for line edit
    layout->addWidget(browseButton);

    setLayout(layout);

    // Connect text changed signal
    connect(fileLineEdit, &QLineEdit::textChanged, this, &FileSelectorWidget::FilePathChanged);
}

void FileSelectorWidget::OnBrowseClicked() {
    QString selectedFile;

    if (selectorType == InputFile) {
        selectedFile = QFileDialog::getOpenFileName(
            this,
            dialogTitle,
            fileLineEdit->text().isEmpty() ? QString() : fileLineEdit->text(),
            fileFilter
        );
    } else {
        selectedFile = QFileDialog::getSaveFileName(
            this,
            dialogTitle,
            fileLineEdit->text().isEmpty() ? QString() : fileLineEdit->text(),
            fileFilter
        );
    }

    if (!selectedFile.isEmpty()) {
        fileLineEdit->setText(selectedFile);
        emit FileSelected(selectedFile);
    }
}

void FileSelectorWidget::SetFilePath(const QString &path) {
    fileLineEdit->setText(path);
}

QString FileSelectorWidget::GetFilePath() const {
    return fileLineEdit->text();
}

void FileSelectorWidget::SetPlaceholder(const QString &text) {
    fileLineEdit->setPlaceholderText(text);
}

void FileSelectorWidget::SetFileFilter(const QString &filter) {
    fileFilter = filter;
}

void FileSelectorWidget::SetDialogTitle(const QString &title) {
    dialogTitle = title;
}

void FileSelectorWidget::SetBrowseEnabled(bool enabled) {
    browseButton->setEnabled(enabled);
}
