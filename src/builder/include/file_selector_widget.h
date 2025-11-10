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

#ifndef FILE_SELECTOR_WIDGET_H
#define FILE_SELECTOR_WIDGET_H

#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

/**
 * @brief Reusable widget for file selection (input or output)
 *
 * This widget encapsulates the common pattern of:
 * - QGroupBox with title
 * - QLineEdit for file path (read-only)
 * - QPushButton for browsing
 *
 * Reduces code duplication across all page classes.
 */
class FileSelectorWidget : public QGroupBox {
    Q_OBJECT

public:
    enum SelectorType {
        InputFile,   // For selecting existing files (QFileDialog::getOpenFileName)
        OutputFile   // For selecting save location (QFileDialog::getSaveFileName)
    };

    /**
     * @brief Construct a file selector widget (simple version)
     * @param title Group box title (e.g., "Input File", "Output File")
     * @param type Whether this is for input or output file selection
     * @param parent Parent widget
     */
    explicit FileSelectorWidget(const QString &title, SelectorType type, QWidget *parent = nullptr);

    /**
     * @brief Construct a file selector widget with full configuration
     * @param title Group box title (e.g., "Input File", "Output File")
     * @param type Whether this is for input or output file selection
     * @param placeholder Placeholder text for the line edit
     * @param fileFilter File dialog filter (e.g., "Video Files (*.mp4);;All Files (*.*)")
     * @param dialogTitle File dialog title
     * @param parent Parent widget
     */
    explicit FileSelectorWidget(const QString &title,
                                SelectorType type,
                                const QString &placeholder,
                                const QString &fileFilter,
                                const QString &dialogTitle,
                                QWidget *parent = nullptr);

    /**
     * @brief Set the file path displayed in the line edit
     */
    void SetFilePath(const QString &path);

    /**
     * @brief Get the current file path
     */
    QString GetFilePath() const;

    /**
     * @brief Set placeholder text for the line edit
     */
    void SetPlaceholder(const QString &text);

    /**
     * @brief Set the file dialog filter
     * @param filter Qt file dialog filter format (e.g., "Video Files (*.mp4 *.avi);;All Files (*.*)")
     */
    void SetFileFilter(const QString &filter);

    /**
     * @brief Set the file dialog title
     */
    void SetDialogTitle(const QString &title);

    /**
     * @brief Enable or disable the browse button
     */
    void SetBrowseEnabled(bool enabled);

    /**
     * @brief Get the line edit widget (for direct access if needed)
     */
    QLineEdit* GetLineEdit() const { return fileLineEdit; }

    /**
     * @brief Get the browse button widget (for direct access if needed)
     */
    QPushButton* GetBrowseButton() const { return browseButton; }

    /**
     * @brief Get the batch button widget (for direct access if needed)
     */
    QPushButton* GetBatchButton() const { return batchButton; }

    /**
     * @brief Enable or disable batch mode button
     */
    void SetBatchEnabled(bool enabled);

    /**
     * @brief Get the list of batch files (if batch mode was used)
     */
    QStringList GetBatchFiles() const { return batchFiles; }

    /**
     * @brief Check if batch mode is active (multiple files selected)
     */
    bool IsBatchMode() const { return !batchFiles.isEmpty(); }

    /**
     * @brief Clear batch files and exit batch mode
     */
    void ClearBatchFiles();

    /**
     * @brief Retranslate UI when language changes
     */
    void RetranslateUi();

signals:
    /**
     * @brief Emitted when user selects a file via the browse button
     * @param filePath The selected file path
     */
    void FileSelected(const QString &filePath);

    /**
     * @brief Emitted when the file path text changes
     */
    void FilePathChanged(const QString &filePath);

    /**
     * @brief Emitted when batch files are selected
     * @param files List of selected files
     */
    void BatchFilesSelected(const QStringList &files);

private slots:
    void OnBrowseClicked();
    void OnBatchClicked();

private:
    void SetupUI();
    QString ConvertFilterToBatchFormat(const QString &qtFilter) const;

    SelectorType selectorType;
    QLineEdit *fileLineEdit;
    QPushButton *browseButton;
    QPushButton *batchButton;
    QString fileFilter;          // Qt dialog filter format: "Video Files (*.mp4);;All Files (*.*)"
    QString dialogTitle;
    QStringList batchFiles;
};

#endif // FILE_SELECTOR_WIDGET_H
