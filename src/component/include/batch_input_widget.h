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

#ifndef BATCH_INPUT_WIDGET_H
#define BATCH_INPUT_WIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QStringList>

/**
 * @brief Widget for batch input file selection
 *
 * Provides two modes:
 * 1. Select multiple files directly
 * 2. Select a directory and filter by extension (.jpg, .png, .mp4, etc.)
 *
 * Features:
 * - Add Files button: Opens file dialog for multiple file selection
 * - Add Directory button: Opens directory dialog
 * - Filter input: Comma-separated extensions (e.g., ".jpg,.png,.mp4")
 * - File list: Shows all selected files
 * - Remove/Clear buttons: Manage the file list
 */
class BatchInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit BatchInputWidget(QWidget *parent = nullptr);
    ~BatchInputWidget();

    // Get selected files
    QStringList GetInputFiles() const;

    // Set file filter for directory mode
    void SetFileFilter(const QString &filter);
    QString GetFileFilter() const;

    // Clear all files
    void Clear();

    // Retranslate UI when language changes
    void RetranslateUi();

signals:
    void FilesChanged(const QStringList &files);

private slots:
    void OnAddFilesClicked();
    void OnAddDirectoryClicked();
    void OnRemoveSelectedClicked();
    void OnClearAllClicked();
    void OnFilterChanged(const QString &text);

private:
    void SetupUI();
    void AddFiles(const QStringList &files);
    void AddDirectory(const QString &dirPath);
    QStringList FilterFilesByExtension(const QStringList &files, const QString &filter);

    // UI Components
    QGroupBox *groupBox;
    QPushButton *addFilesButton;
    QPushButton *addDirectoryButton;
    QPushButton *removeSelectedButton;
    QPushButton *clearAllButton;
    QListWidget *fileListWidget;
    QLineEdit *filterLineEdit;
    QLabel *filterLabel;
    QLabel *fileCountLabel;

    QString currentFilter;  // e.g., ".jpg,.png,.mp4"
};

#endif // BATCH_INPUT_WIDGET_H
