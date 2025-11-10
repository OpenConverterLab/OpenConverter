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

#ifndef BATCH_FILE_DIALOG_H
#define BATCH_FILE_DIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QStringList>
#include "filter_tag_widget.h"

/**
 * @brief Small dialog for batch file selection
 *
 * This dialog allows users to:
 * 1. Add multiple files
 * 2. Add files from a directory with filter
 * 3. Remove selected files
 * 4. Clear all files
 */
class BatchFileDialog : public QDialog {
    Q_OBJECT

public:
    explicit BatchFileDialog(QWidget *parent = nullptr);
    ~BatchFileDialog() override = default;

    /**
     * @brief Set the file filter for file/directory selection
     * @param filter Comma-separated extensions (e.g., "*.mp4,*.avi,*.mkv")
     */
    void SetFileFilter(const QString &filter);

    /**
     * @brief Get the list of selected files
     */
    QStringList GetSelectedFiles() const;

    /**
     * @brief Set initial files
     */
    void SetFiles(const QStringList &files);

    /**
     * @brief Retranslate UI when language changes
     */
    void RetranslateUi();

private slots:
    void OnAddFilesClicked();
    void OnAddDirectoryClicked();
    void OnRemoveSelectedClicked();
    void OnClearAllClicked();
    void OnOkClicked();
    void OnCancelClicked();

private:
    void SetupUI();
    QString ConvertFilterToDialogFormat(const QString &filter);

    QListWidget *fileListWidget;
    QPushButton *addFilesButton;
    QPushButton *addDirectoryButton;
    QPushButton *removeSelectedButton;
    QPushButton *clearAllButton;
    QLabel *filterLabel;
    FilterTagWidget *filterTagWidget;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QString fileFilter;  // Format: "*.mp4,*.avi,*.mkv"
    QStringList selectedFiles;
};

#endif // BATCH_FILE_DIALOG_H
