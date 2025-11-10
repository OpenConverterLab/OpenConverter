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

#include "../include/batch_mode_helper.h"
#include "../include/batch_queue.h"
#include <QMessageBox>

BatchModeHelper::BatchModeHelper(FileSelectorWidget *inputFileSelector,
                                 BatchOutputWidget *batchOutputWidget,
                                 QPushButton *actionButton,
                                 const QString &singleModeText,
                                 const QString &batchModeText,
                                 QObject *parent)
    : QObject(parent),
      inputFileSelector(inputFileSelector),
      batchOutputWidget(batchOutputWidget),
      actionButton(actionButton),
      singleModeText(singleModeText),
      batchModeText(batchModeText),
      singleOutputWidget(nullptr),
      encodeParameterCreator(nullptr) {

    // Connect to batch files selected signal
    connect(inputFileSelector, &FileSelectorWidget::BatchFilesSelected,
            this, &BatchModeHelper::OnBatchFilesSelected);
}

void BatchModeHelper::SetSingleOutputWidget(QWidget *widget) {
    singleOutputWidget = widget;
}

void BatchModeHelper::SetEncodeParameterCreator(std::function<EncodeParameter*()> creator) {
    encodeParameterCreator = creator;
}

bool BatchModeHelper::IsBatchMode() const {
    return inputFileSelector->IsBatchMode();
}

bool BatchModeHelper::AddToQueue(const QString &outputFormat) {
    if (!IsBatchMode()) {
        return false;
    }

    QStringList inputFiles = inputFileSelector->GetBatchFiles();
    if (inputFiles.isEmpty()) {
        QMessageBox::warning(qobject_cast<QWidget*>(parent()), QObject::tr("No Files"),
                           QObject::tr("Please select files to process."));
        return false;
    }

    if (!encodeParameterCreator) {
        QMessageBox::critical(qobject_cast<QWidget*>(parent()), QObject::tr("Error"),
                            QObject::tr("Encode parameter creator not set."));
        return false;
    }

    // Create batch items
    QList<BatchItem*> items;
    for (const QString &inputFile : inputFiles) {
        // Generate output path
        QString outputPath = batchOutputWidget->GenerateOutputPath(inputFile, outputFormat);

        // Create batch item
        BatchItem *item = new BatchItem(inputFile, outputPath);

        // Create encode parameter for this item
        EncodeParameter *encodeParam = encodeParameterCreator();
        item->SetEncodeParameter(encodeParam);

        items.append(item);
    }

    // Add items to queue
    BatchQueue::Instance()->AddItems(items);

    // Show confirmation
    QMessageBox::information(qobject_cast<QWidget*>(parent()), QObject::tr("Added to Queue"),
                           QObject::tr("Added %1 file(s) to the batch queue.").arg(items.count()));

    // Clear batch files and reset UI
    inputFileSelector->ClearBatchFiles();
    batchOutputWidget->setVisible(false);
    if (singleOutputWidget) {
        singleOutputWidget->setVisible(true);
    }
    actionButton->setText(singleModeText);
    actionButton->setEnabled(false);

    return true;
}

void BatchModeHelper::OnBatchFilesSelected(const QStringList &files) {
    if (files.isEmpty()) {
        // Batch mode disabled - show single output widget, hide batch output widget
        batchOutputWidget->setVisible(false);
        if (singleOutputWidget) {
            singleOutputWidget->setVisible(true);
        }
        actionButton->setText(singleModeText);
        actionButton->setEnabled(!inputFileSelector->GetFilePath().isEmpty());
    } else {
        // Batch mode enabled - hide single output widget, show batch output widget
        if (singleOutputWidget) {
            singleOutputWidget->setVisible(false);
        }
        batchOutputWidget->setVisible(true);
        actionButton->setText(batchModeText);
        actionButton->setEnabled(true);
    }
}
