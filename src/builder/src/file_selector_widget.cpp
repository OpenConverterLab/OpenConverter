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
#include "../include/batch_file_dialog.h"
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

    // Add batch button (only for input files)
    batchButton = new QPushButton(tr("Batch"), this);
    connect(batchButton, &QPushButton::clicked, this, &FileSelectorWidget::OnBatchClicked);
    batchButton->setVisible(selectorType == InputFile);  // Only show for input files

    layout->addWidget(fileLineEdit, 1);  // Stretch factor 1 for line edit
    layout->addWidget(browseButton);
    layout->addWidget(batchButton);

    setLayout(layout);

    // Connect text changed signal
    connect(fileLineEdit, &QLineEdit::textChanged, this, &FileSelectorWidget::FilePathChanged);
}

void FileSelectorWidget::OnBrowseClicked() {
    QString selectedFile;

    // Get starting directory (don't use batch mode text as path)
    QString startPath;
    if (!IsBatchMode() && !fileLineEdit->text().isEmpty()) {
        startPath = fileLineEdit->text();
    }

    if (selectorType == InputFile) {
        selectedFile = QFileDialog::getOpenFileName(
            this,
            dialogTitle,
            startPath,
            fileFilter
        );
    } else {
        selectedFile = QFileDialog::getSaveFileName(
            this,
            dialogTitle,
            startPath,
            fileFilter
        );
    }

    if (!selectedFile.isEmpty()) {
        // Clear batch mode when selecting a single file
        batchFiles.clear();
        fileLineEdit->setText(selectedFile);
        emit FileSelected(selectedFile);
    }
}

void FileSelectorWidget::SetFilePath(const QString &path) {
    fileLineEdit->setText(path);

    // Clear batch files when setting a single file path
    if (!path.isEmpty()) {
        batchFiles.clear();
    }
}

QString FileSelectorWidget::GetFilePath() const {
    return fileLineEdit->text();
}

void FileSelectorWidget::SetPlaceholder(const QString &text) {
    fileLineEdit->setPlaceholderText(text);
}

void FileSelectorWidget::SetBatchEnabled(bool enabled) {
    batchButton->setVisible(enabled && selectorType == InputFile);
}

void FileSelectorWidget::OnBatchClicked() {
    BatchFileDialog *dialog = new BatchFileDialog(this);
    dialog->SetFileFilter(ConvertFilterToBatchFormat(fileFilter));
    dialog->SetFiles(batchFiles);

    if (dialog->exec() == QDialog::Accepted) {
        batchFiles = dialog->GetSelectedFiles();

        if (!batchFiles.isEmpty()) {
            // Update line edit to show count
            fileLineEdit->setText(tr("%1 files selected").arg(batchFiles.count()));
            emit BatchFilesSelected(batchFiles);
        } else {
            fileLineEdit->clear();
        }
    }

    dialog->deleteLater();
}

void FileSelectorWidget::ClearBatchFiles() {
    batchFiles.clear();
    fileLineEdit->clear();
}

void FileSelectorWidget::SetFileFilter(const QString &filter) {
    fileFilter = filter;
}

QString FileSelectorWidget::ConvertFilterToBatchFormat(const QString &qtFilter) const {
    // Convert Qt filter format to batch format
    // Input: "Video Files (*.mp4 *.avi *.mkv);;All Files (*.*)"
    // Output: "*.mp4,*.avi,*.mkv"

    if (qtFilter.isEmpty()) {
        return QString();
    }

    // Extract first filter group (before ";;")
    QString firstGroup = qtFilter.split(";;").first();

    // Extract extensions from parentheses
    int start = firstGroup.indexOf('(');
    int end = firstGroup.indexOf(')');
    if (start == -1 || end == -1) {
        return QString();
    }

    QString extensions = firstGroup.mid(start + 1, end - start - 1);

    // Convert space-separated to comma-separated
    QStringList extList = extensions.split(' ', Qt::SkipEmptyParts);

    // Filter out "All Files (*.*)" pattern
    extList.removeAll("*.*");

    return extList.join(',');
}

void FileSelectorWidget::SetDialogTitle(const QString &title) {
    dialogTitle = title;
}

void FileSelectorWidget::SetBrowseEnabled(bool enabled) {
    browseButton->setEnabled(enabled);
}

void FileSelectorWidget::RetranslateUi() {
    browseButton->setText(tr("Browse..."));
    batchButton->setText(tr("Batch"));

    // Update batch file count display if in batch mode
    if (!batchFiles.isEmpty()) {
        fileLineEdit->setText(tr("%1 files selected").arg(batchFiles.count()));
    }
}
