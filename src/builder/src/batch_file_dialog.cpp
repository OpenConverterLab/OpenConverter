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

#include "../include/batch_file_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

BatchFileDialog::BatchFileDialog(QWidget *parent)
    : QDialog(parent), fileFilter("*.*") {
    SetupUI();
    setWindowTitle(tr("Batch File Selection"));
    resize(600, 400);
}

void BatchFileDialog::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Filter section
    filterLabel = new QLabel(tr("File Filter:"), this);
    mainLayout->addWidget(filterLabel);

    filterTagWidget = new FilterTagWidget(this);
    connect(filterTagWidget, &FilterTagWidget::FilterChanged, this, [this](const QString &filter) {
        fileFilter = filter;
    });
    mainLayout->addWidget(filterTagWidget);

    // Add buttons
    QHBoxLayout *addButtonLayout = new QHBoxLayout();
    addFilesButton = new QPushButton(tr("Add Files"), this);
    addDirectoryButton = new QPushButton(tr("Add Directory"), this);
    connect(addFilesButton, &QPushButton::clicked, this, &BatchFileDialog::OnAddFilesClicked);
    connect(addDirectoryButton, &QPushButton::clicked, this, &BatchFileDialog::OnAddDirectoryClicked);
    addButtonLayout->addWidget(addFilesButton);
    addButtonLayout->addWidget(addDirectoryButton);
    addButtonLayout->addStretch();
    mainLayout->addLayout(addButtonLayout);

    // File list
    fileListWidget = new QListWidget(this);
    fileListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mainLayout->addWidget(fileListWidget);

    // List management buttons
    QHBoxLayout *listButtonLayout = new QHBoxLayout();
    removeSelectedButton = new QPushButton(tr("Remove Selected"), this);
    clearAllButton = new QPushButton(tr("Clear All"), this);
    connect(removeSelectedButton, &QPushButton::clicked, this, &BatchFileDialog::OnRemoveSelectedClicked);
    connect(clearAllButton, &QPushButton::clicked, this, &BatchFileDialog::OnClearAllClicked);
    listButtonLayout->addWidget(removeSelectedButton);
    listButtonLayout->addWidget(clearAllButton);
    listButtonLayout->addStretch();
    mainLayout->addLayout(listButtonLayout);

    // Dialog buttons
    QHBoxLayout *dialogButtonLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("OK"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);
    connect(okButton, &QPushButton::clicked, this, &BatchFileDialog::OnOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &BatchFileDialog::OnCancelClicked);
    dialogButtonLayout->addStretch();
    dialogButtonLayout->addWidget(okButton);
    dialogButtonLayout->addWidget(cancelButton);
    mainLayout->addLayout(dialogButtonLayout);

    setLayout(mainLayout);
}

void BatchFileDialog::SetFileFilter(const QString &filter) {
    fileFilter = filter;
    filterTagWidget->SetFilter(filter);
}

QStringList BatchFileDialog::GetSelectedFiles() const {
    return selectedFiles;
}

void BatchFileDialog::SetFiles(const QStringList &files) {
    selectedFiles = files;
    fileListWidget->clear();
    for (const QString &file : files) {
        fileListWidget->addItem(file);
    }
}

QString BatchFileDialog::ConvertFilterToDialogFormat(const QString &filter) {
    // Convert "*.mp4,*.avi,*.mkv" to "Media Files (*.mp4 *.avi *.mkv);;All Files (*.*)"
    QString extensions = filter;
    extensions.replace(",", " ");
    return QString("Files (%1);;All Files (*.*)").arg(extensions);
}

void BatchFileDialog::OnAddFilesClicked() {
    QString filter = filterTagWidget->GetFilter();
    if (filter.isEmpty()) {
        filter = fileFilter;
    }
    QString dialogFilter = ConvertFilterToDialogFormat(filter);

    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Files"),
        QString(),
        dialogFilter
    );

    if (!files.isEmpty()) {
        for (const QString &file : files) {
            if (!selectedFiles.contains(file)) {
                selectedFiles.append(file);
                fileListWidget->addItem(file);
            }
        }
    }
}

void BatchFileDialog::OnAddDirectoryClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Directory"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        QString filter = filterTagWidget->GetFilter();
        if (filter.isEmpty()) {
            filter = fileFilter;
        }
        QStringList filters = filter.split(",", Qt::SkipEmptyParts);

        QDir directory(dir);
        QStringList files = directory.entryList(filters, QDir::Files);

        for (const QString &file : files) {
            QString fullPath = directory.absoluteFilePath(file);
            if (!selectedFiles.contains(fullPath)) {
                selectedFiles.append(fullPath);
                fileListWidget->addItem(fullPath);
            }
        }

        if (files.isEmpty()) {
            QMessageBox::information(this, tr("No Files"),
                                   tr("No files matching the filter were found in the selected directory."));
        }
    }
}

void BatchFileDialog::OnRemoveSelectedClicked() {
    QList<QListWidgetItem*> items = fileListWidget->selectedItems();
    for (QListWidgetItem *item : items) {
        QString filePath = item->text();
        selectedFiles.removeAll(filePath);
        delete fileListWidget->takeItem(fileListWidget->row(item));
    }
}

void BatchFileDialog::OnClearAllClicked() {
    selectedFiles.clear();
    fileListWidget->clear();
}

void BatchFileDialog::OnOkClicked() {
    accept();
}

void BatchFileDialog::OnCancelClicked() {
    reject();
}

void BatchFileDialog::RetranslateUi() {
    setWindowTitle(tr("Batch File Selection"));
    filterLabel->setText(tr("File Filter:"));
    filterTagWidget->RetranslateUi();
    addFilesButton->setText(tr("Add Files"));
    addDirectoryButton->setText(tr("Add Directory"));
    removeSelectedButton->setText(tr("Remove Selected"));
    clearAllButton->setText(tr("Clear All"));
    okButton->setText(tr("OK"));
    cancelButton->setText(tr("Cancel"));
}
