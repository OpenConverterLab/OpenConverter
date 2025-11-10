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

#ifndef BATCH_QUEUE_DIALOG_H
#define BATCH_QUEUE_DIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "batch_item.h"
#include "batch_queue.h"
#include "../../common/include/process_parameter.h"

/**
 * @brief Dialog to display and manage the batch processing queue
 *
 * Shows a table with columns:
 * - Status (icon + text: Waiting, Processing, Finished, Failed)
 * - Input File (full path)
 * - Output File (full path)
 * - Progress (percentage for processing items)
 *
 * Features:
 * - Real-time updates as items are processed
 * - Remove selected items
 * - Clear finished/failed items
 * - Clear all items
 * - Start/Stop batch processing
 * - Summary statistics (total, waiting, processing, finished, failed)
 */
class BatchQueueDialog : public QDialog, public ProcessObserver {
    Q_OBJECT

public:
    explicit BatchQueueDialog(QWidget *parent = nullptr);
    ~BatchQueueDialog();

    // Refresh the queue display
    void RefreshQueue();

private slots:
    void OnItemAdded(int index);
    void OnItemRemoved(int index);
    void OnItemStatusChanged(int index, BatchItemStatus status);
    void OnItemProgressChanged(int index, double progress);
    void OnQueueCleared();

    void OnStartClicked();
    void OnStopClicked();
    void OnRemoveSelectedClicked();
    void OnClearFinishedClicked();
    void OnClearAllClicked();
    void OnCloseClicked();

    void OnConversionFinished(bool success);
    void ProcessNextItem();

    // ProcessObserver interface
    void on_process_update(double progress) override;
    void on_time_update(double timeRequired) override;

private:
    void SetupUI();
    void UpdateStatistics();
    void AddItemToTable(BatchItem *item, int row);
    void UpdateItemInTable(int row, BatchItem *item);
    QString GetStatusIcon(BatchItemStatus status) const;
    QColor GetStatusColor(BatchItemStatus status) const;

    // UI Components
    QTableWidget *queueTable;
    QLabel *statisticsLabel;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *removeSelectedButton;
    QPushButton *clearFinishedButton;
    QPushButton *clearAllButton;
    QPushButton *closeButton;

    BatchQueue *batchQueue;

    bool isProcessing;
    int currentItemIndex;
};

#endif // BATCH_QUEUE_DIALOG_H
