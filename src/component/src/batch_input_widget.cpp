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

#include "batch_input_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

BatchInputWidget::BatchInputWidget(QWidget *parent)
    : QWidget(parent), currentFilter("") {
    SetupUI();
}

BatchInputWidget::~BatchInputWidget() {
}

void BatchInputWidget::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Group box
    groupBox = new QGroupBox(tr("Batch Input"), this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    // Button row
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addFilesButton = new QPushButton(tr("Add Files..."), this);
    addDirectoryButton = new QPushButton(tr("Add Directory..."), this);
    removeSelectedButton = new QPushButton(tr("Remove Selected"), this);
    clearAllButton = new QPushButton(tr("Clear All"), this);

    buttonLayout->addWidget(addFilesButton);
    buttonLayout->addWidget(addDirectoryButton);
    buttonLayout->addWidget(removeSelectedButton);
    buttonLayout->addWidget(clearAllButton);
    buttonLayout->addStretch();

    groupLayout->addLayout(buttonLayout);

    // Filter row
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLabel = new QLabel(tr("File Filter (e.g., .jpg,.png,.mp4):"), this);
    filterLineEdit = new QLineEdit(this);
    filterLineEdit->setPlaceholderText(tr("Leave empty for all files"));

    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filterLineEdit);

    groupLayout->addLayout(filterLayout);

    // File list
    fileListWidget = new QListWidget(this);
    fileListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileListWidget->setMinimumHeight(150);
    groupLayout->addWidget(fileListWidget);

    // File count label
    fileCountLabel = new QLabel(tr("Files: 0"), this);
    groupLayout->addWidget(fileCountLabel);

    mainLayout->addWidget(groupBox);

    // Connect signals
    connect(addFilesButton, &QPushButton::clicked, this, &BatchInputWidget::OnAddFilesClicked);
    connect(addDirectoryButton, &QPushButton::clicked, this, &BatchInputWidget::OnAddDirectoryClicked);
    connect(removeSelectedButton, &QPushButton::clicked, this, &BatchInputWidget::OnRemoveSelectedClicked);
    connect(clearAllButton, &QPushButton::clicked, this, &BatchInputWidget::OnClearAllClicked);
    connect(filterLineEdit, &QLineEdit::textChanged, this, &BatchInputWidget::OnFilterChanged);
}

QStringList BatchInputWidget::GetInputFiles() const {
    QStringList files;
    for (int i = 0; i < fileListWidget->count(); ++i) {
        files.append(fileListWidget->item(i)->text());
    }
    return files;
}

void BatchInputWidget::SetFileFilter(const QString &filter) {
    currentFilter = filter;
    filterLineEdit->setText(filter);
}

QString BatchInputWidget::GetFileFilter() const {
    return currentFilter;
}

void BatchInputWidget::Clear() {
    fileListWidget->clear();
    fileCountLabel->setText(tr("Files: 0"));
    emit FilesChanged(QStringList());
}

void BatchInputWidget::OnAddFilesClicked() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Files"),
        QString(),
        tr("All Files (*.*)")
    );

    if (!files.isEmpty()) {
        AddFiles(files);
    }
}

void BatchInputWidget::OnAddDirectoryClicked() {
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select Directory"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dirPath.isEmpty()) {
        AddDirectory(dirPath);
    }
}

void BatchInputWidget::OnRemoveSelectedClicked() {
    QList<QListWidgetItem*> selectedItems = fileListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems) {
        delete fileListWidget->takeItem(fileListWidget->row(item));
    }

    fileCountLabel->setText(tr("Files: %1").arg(fileListWidget->count()));
    emit FilesChanged(GetInputFiles());
}

void BatchInputWidget::OnClearAllClicked() {
    Clear();
}

void BatchInputWidget::OnFilterChanged(const QString &text) {
    currentFilter = text.trimmed();
}

void BatchInputWidget::AddFiles(const QStringList &files) {
    QStringList filteredFiles = FilterFilesByExtension(files, currentFilter);

    for (const QString &file : filteredFiles) {
        // Check if file already exists in list
        bool exists = false;
        for (int i = 0; i < fileListWidget->count(); ++i) {
            if (fileListWidget->item(i)->text() == file) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            fileListWidget->addItem(file);
        }
    }

    fileCountLabel->setText(tr("Files: %1").arg(fileListWidget->count()));
    emit FilesChanged(GetInputFiles());
}

void BatchInputWidget::AddDirectory(const QString &dirPath) {
    QDir dir(dirPath);
    QStringList allFiles;

    // Get all files in directory (non-recursive for now)
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &fileInfo : fileInfoList) {
        allFiles.append(fileInfo.absoluteFilePath());
    }

    if (allFiles.isEmpty()) {
        QMessageBox::information(this, tr("No Files"),
                                tr("No files found in the selected directory."));
        return;
    }

    AddFiles(allFiles);
}

QStringList BatchInputWidget::FilterFilesByExtension(const QStringList &files, const QString &filter) {
    if (filter.isEmpty()) {
        return files;
    }

    // Parse filter: ".jpg,.png,.mp4" -> [".jpg", ".png", ".mp4"]
    QStringList extensions = filter.split(',', Qt::SkipEmptyParts);
    for (QString &ext : extensions) {
        ext = ext.trimmed().toLower();
        if (!ext.startsWith('.')) {
            ext = '.' + ext;
        }
    }

    QStringList filteredFiles;
    for (const QString &file : files) {
        QFileInfo fileInfo(file);
        QString fileExt = '.' + fileInfo.suffix().toLower();

        if (extensions.contains(fileExt)) {
            filteredFiles.append(file);
        }
    }

    return filteredFiles;
}

void BatchInputWidget::RetranslateUi() {
    groupBox->setTitle(tr("Batch Input"));
    addFilesButton->setText(tr("Add Files..."));
    addDirectoryButton->setText(tr("Add Directory..."));
    removeSelectedButton->setText(tr("Remove Selected"));
    clearAllButton->setText(tr("Clear All"));
    filterLabel->setText(tr("File Filter (e.g., .jpg,.png,.mp4):"));
    filterLineEdit->setPlaceholderText(tr("Leave empty for all files"));

    // Update file count label
    int fileCount = fileListWidget->count();
    if (fileCount == 0) {
        fileCountLabel->setText(tr("Files: 0"));
    } else {
        fileCountLabel->setText(tr("Files: %1").arg(fileCount));
    }
}
