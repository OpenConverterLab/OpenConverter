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

#ifndef BATCH_MODE_HELPER_H
#define BATCH_MODE_HELPER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <functional>
#include "file_selector_widget.h"
#include "batch_output_widget.h"
#include "batch_item.h"
#include "../../common/include/encode_parameter.h"

/**
 * @brief Helper class to manage batch mode functionality for pages
 *
 * This class abstracts the common batch mode logic that's shared across
 * multiple pages (TranscodePage, CompressPicturePage, ExtractAudioPage, RemuxPage).
 *
 * Features:
 * - Connects to FileSelectorWidget's BatchFilesSelected signal
 * - Shows/hides BatchOutputWidget based on batch mode state
 * - Updates action button text (e.g., "Transcode" vs "Add to Queue")
 * - Creates batch items with encode parameters
 * - Adds items to BatchQueue
 *
 * Usage:
 *   BatchModeHelper *helper = new BatchModeHelper(
 *       inputFileSelector, batchOutputWidget, actionButton,
 *       "Transcode", "Add to Queue", this);
 *
 *   helper->SetEncodeParameterCreator([this]() {
 *       return CreateEncodeParameter();
 *   });
 *
 *   connect(actionButton, &QPushButton::clicked, [this, helper]() {
 *       if (helper->IsBatchMode()) {
 *           helper->AddToQueue("mp4");
 *       } else {
 *           // Single file mode
 *       }
 *   });
 */
class BatchModeHelper : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param inputFileSelector Input file selector widget
     * @param batchOutputWidget Batch output widget
     * @param actionButton Action button (e.g., Transcode, Extract, etc.)
     * @param singleModeText Button text for single file mode
     * @param batchModeText Button text for batch mode
     * @param parent Parent QObject
     */
    explicit BatchModeHelper(FileSelectorWidget *inputFileSelector,
                            BatchOutputWidget *batchOutputWidget,
                            QPushButton *actionButton,
                            const QString &singleModeText,
                            const QString &batchModeText,
                            QObject *parent = nullptr);

    /**
     * @brief Set the single output widget to hide/show when switching modes
     * @param singleOutputWidget Widget to hide in batch mode (e.g., output file selector)
     */
    void SetSingleOutputWidget(QWidget *singleOutputWidget);

    /**
     * @brief Set the function to create encode parameters
     * @param creator Function that creates and returns EncodeParameter*
     */
    void SetEncodeParameterCreator(std::function<EncodeParameter*()> creator);

    /**
     * @brief Check if batch mode is active
     * @return true if batch mode is active, false otherwise
     */
    bool IsBatchMode() const;

    /**
     * @brief Add batch files to queue
     * @param outputFormat Output format extension (e.g., "mp4", "jpg")
     * @return true if items were added successfully, false otherwise
     */
    bool AddToQueue(const QString &outputFormat);

private slots:
    void OnBatchFilesSelected(const QStringList &files);

private:
    FileSelectorWidget *inputFileSelector;
    BatchOutputWidget *batchOutputWidget;
    QPushButton *actionButton;
    QString singleModeText;
    QString batchModeText;
    QWidget *singleOutputWidget;  // Optional: widget to hide in batch mode

    std::function<EncodeParameter*()> encodeParameterCreator;
};

#endif // BATCH_MODE_HELPER_H
