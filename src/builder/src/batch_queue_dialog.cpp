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

#include "../include/batch_queue_dialog.h"
#include "../../engine/include/converter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QThread>
#include <QMetaObject>

BatchQueueDialog::BatchQueueDialog(QWidget *parent)
    : QDialog(parent),
      isProcessing(false),
      currentItemIndex(-1) {
    batchQueue = BatchQueue::Instance();

    SetupUI();
    RefreshQueue();

    // Connect to batch queue signals with Qt::QueuedConnection to avoid deadlocks
    // (signals are emitted while mutex is locked, slots may need to lock mutex again)
    connect(batchQueue, &BatchQueue::ItemAdded, this, &BatchQueueDialog::OnItemAdded, Qt::QueuedConnection);
    connect(batchQueue, &BatchQueue::ItemRemoved, this, &BatchQueueDialog::OnItemRemoved, Qt::QueuedConnection);
    connect(batchQueue, &BatchQueue::ItemStatusChanged, this, &BatchQueueDialog::OnItemStatusChanged, Qt::QueuedConnection);
    connect(batchQueue, &BatchQueue::ItemProgressChanged, this, &BatchQueueDialog::OnItemProgressChanged, Qt::QueuedConnection);
    connect(batchQueue, &BatchQueue::QueueCleared, this, &BatchQueueDialog::OnQueueCleared, Qt::QueuedConnection);
}

BatchQueueDialog::~BatchQueueDialog() {
}

void BatchQueueDialog::SetupUI() {
    setWindowTitle(tr("Batch Processing Queue"));
    setMinimumSize(800, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Statistics label
    statisticsLabel = new QLabel(this);
    statisticsLabel->setStyleSheet("QLabel { font-weight: bold; padding: 5px; }");
    mainLayout->addWidget(statisticsLabel);

    // Queue table
    queueTable = new QTableWidget(this);
    queueTable->setColumnCount(4);
    queueTable->setHorizontalHeaderLabels({tr("Status"), tr("Input File"), tr("Output File"), tr("Progress")});
    queueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    queueTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    queueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    queueTable->horizontalHeader()->setStretchLastSection(false);
    queueTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    queueTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    queueTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    queueTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    queueTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(queueTable);

    // Button row
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    startButton = new QPushButton(tr("▶ Start"), this);
    stopButton = new QPushButton(tr("⏹ Stop"), this);
    stopButton->setEnabled(false);
    removeSelectedButton = new QPushButton(tr("Remove Selected"), this);
    clearFinishedButton = new QPushButton(tr("Clear Finished"), this);
    clearAllButton = new QPushButton(tr("Clear All"), this);
    closeButton = new QPushButton(tr("Close"), this);

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(removeSelectedButton);
    buttonLayout->addWidget(clearFinishedButton);
    buttonLayout->addWidget(clearAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);

    // Connect button signals
    connect(startButton, &QPushButton::clicked, this, &BatchQueueDialog::OnStartClicked);
    connect(stopButton, &QPushButton::clicked, this, &BatchQueueDialog::OnStopClicked);
    connect(removeSelectedButton, &QPushButton::clicked, this, &BatchQueueDialog::OnRemoveSelectedClicked);
    connect(clearFinishedButton, &QPushButton::clicked, this, &BatchQueueDialog::OnClearFinishedClicked);
    connect(clearAllButton, &QPushButton::clicked, this, &BatchQueueDialog::OnClearAllClicked);
    connect(closeButton, &QPushButton::clicked, this, &BatchQueueDialog::OnCloseClicked);
}

void BatchQueueDialog::RefreshQueue() {
    queueTable->setRowCount(0);

    QList<BatchItem*> items = batchQueue->GetAllItems();
    for (int i = 0; i < items.size(); ++i) {
        queueTable->insertRow(i);
        AddItemToTable(items[i], i);
    }

    UpdateStatistics();
}

void BatchQueueDialog::UpdateStatistics() {
    int total = batchQueue->GetCount();
    int waiting = batchQueue->GetWaitingCount();
    int processing = batchQueue->GetProcessingCount();
    int finished = batchQueue->GetFinishedCount();
    int failed = batchQueue->GetFailedCount();

    QString stats = tr("Total: %1 | Waiting: %2 | Processing: %3 | Finished: %4 | Failed: %5")
                    .arg(total).arg(waiting).arg(processing).arg(finished).arg(failed);
    statisticsLabel->setText(stats);
}

void BatchQueueDialog::AddItemToTable(BatchItem *item, int row) {
    if (!item) return;

    // Status column
    QTableWidgetItem *statusItem = new QTableWidgetItem(GetStatusIcon(item->GetStatus()) + " " + item->GetStatusString());
    statusItem->setForeground(GetStatusColor(item->GetStatus()));
    queueTable->setItem(row, 0, statusItem);

    // Input file column
    QFileInfo inputInfo(item->GetInputPath());
    QTableWidgetItem *inputItem = new QTableWidgetItem(inputInfo.fileName());
    inputItem->setToolTip(item->GetInputPath());
    queueTable->setItem(row, 1, inputItem);

    // Output file column
    QFileInfo outputInfo(item->GetOutputPath());
    QTableWidgetItem *outputItem = new QTableWidgetItem(outputInfo.fileName());
    outputItem->setToolTip(item->GetOutputPath());
    queueTable->setItem(row, 2, outputItem);

    // Progress column
    QString progressText;
    if (item->GetStatus() == BatchItemStatus::Processing) {
        progressText = QString::number(static_cast<int>(item->GetProgress())) + "%";
    } else if (item->GetStatus() == BatchItemStatus::Finished) {
        progressText = "100%";
    } else if (item->GetStatus() == BatchItemStatus::Failed) {
        progressText = "Failed";
    } else {
        progressText = "-";
    }
    QTableWidgetItem *progressItem = new QTableWidgetItem(progressText);
    queueTable->setItem(row, 3, progressItem);
}

void BatchQueueDialog::UpdateItemInTable(int row, BatchItem *item) {
    if (!item || row < 0 || row >= queueTable->rowCount()) return;

    // Update status
    queueTable->item(row, 0)->setText(GetStatusIcon(item->GetStatus()) + " " + item->GetStatusString());
    queueTable->item(row, 0)->setForeground(GetStatusColor(item->GetStatus()));

    // Update progress
    QString progressText;
    if (item->GetStatus() == BatchItemStatus::Processing) {
        progressText = QString::number(static_cast<int>(item->GetProgress())) + "%";
    } else if (item->GetStatus() == BatchItemStatus::Finished) {
        progressText = "100%";
    } else if (item->GetStatus() == BatchItemStatus::Failed) {
        progressText = "Failed";
    } else {
        progressText = "-";
    }
    queueTable->item(row, 3)->setText(progressText);
}

QString BatchQueueDialog::GetStatusIcon(BatchItemStatus status) const {
    switch (status) {
        case BatchItemStatus::Waiting:
            return "⏳";
        case BatchItemStatus::Processing:
            return "▶";
        case BatchItemStatus::Finished:
            return "✓";
        case BatchItemStatus::Failed:
            return "✗";
        default:
            return "?";
    }
}

QColor BatchQueueDialog::GetStatusColor(BatchItemStatus status) const {
    switch (status) {
        case BatchItemStatus::Waiting:
            return QColor(128, 128, 128);  // Gray
        case BatchItemStatus::Processing:
            return QColor(0, 122, 204);    // Blue
        case BatchItemStatus::Finished:
            return QColor(0, 128, 0);      // Green
        case BatchItemStatus::Failed:
            return QColor(204, 0, 0);      // Red
        default:
            return QColor(0, 0, 0);        // Black
    }
}

void BatchQueueDialog::OnItemAdded(int index) {
    RefreshQueue();
}

void BatchQueueDialog::OnItemRemoved(int index) {
    RefreshQueue();
}

void BatchQueueDialog::OnItemStatusChanged(int index, BatchItemStatus status) {
    BatchItem *item = batchQueue->GetItem(index);
    if (item && index < queueTable->rowCount()) {
        UpdateItemInTable(index, item);
        UpdateStatistics();
    }
}

void BatchQueueDialog::OnItemProgressChanged(int index, double progress) {
    BatchItem *item = batchQueue->GetItem(index);
    if (item && index < queueTable->rowCount()) {
        UpdateItemInTable(index, item);
    }
}

void BatchQueueDialog::OnQueueCleared() {
    RefreshQueue();
}

void BatchQueueDialog::OnRemoveSelectedClicked() {
    QList<QTableWidgetItem*> selectedItems = queueTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    // Get unique row indices
    QSet<int> rows;
    for (QTableWidgetItem *item : selectedItems) {
        rows.insert(item->row());
    }

    // Check if any selected item is currently processing
    QList<BatchItem*> allItems = batchQueue->GetAllItems();
    for (int row : rows) {
        if (row >= 0 && row < allItems.size()) {
            if (allItems[row]->GetStatus() == BatchItemStatus::Processing) {
                QMessageBox::warning(this, tr("Cannot Remove"),
                                   tr("Cannot remove items that are currently being processed."));
                return;
            }
        }
    }

    // Remove from highest index to lowest to avoid index shifting
    QList<int> rowList = rows.values();
    std::sort(rowList.begin(), rowList.end(), std::greater<int>());

    for (int row : rowList) {
        batchQueue->RemoveItem(row);
    }
}

void BatchQueueDialog::OnClearFinishedClicked() {
    QList<BatchItem*> items = batchQueue->GetAllItems();
    for (int i = items.size() - 1; i >= 0; --i) {
        BatchItemStatus status = items[i]->GetStatus();
        if (status == BatchItemStatus::Finished || status == BatchItemStatus::Failed) {
            batchQueue->RemoveItem(i);
        }
    }
}

void BatchQueueDialog::OnClearAllClicked() {
    if (batchQueue->IsProcessing()) {
        QMessageBox::warning(this, tr("Cannot Clear"),
                           tr("Cannot clear queue while items are being processed."));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Clear All"),
        tr("Are you sure you want to clear all items from the queue?"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        batchQueue->Clear();
    }
}

void BatchQueueDialog::OnCloseClicked() {
    close();
}

void BatchQueueDialog::OnStartClicked() {
    if (!batchQueue->HasWaitingItems()) {
        QMessageBox::information(this, tr("No Items"),
                               tr("There are no items in the queue to process."));
        return;
    }

    isProcessing = true;
    startButton->setEnabled(false);
    stopButton->setEnabled(true);

    ProcessNextItem();
}

void BatchQueueDialog::OnStopClicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Stop Processing"),
        tr("Are you sure you want to stop batch processing?"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        isProcessing = false;
        startButton->setEnabled(true);
        stopButton->setEnabled(false);
    }
}

void BatchQueueDialog::ProcessNextItem() {
    if (!isProcessing) {
        return;
    }

    // Get next waiting item
    BatchItem *item = batchQueue->GetNextWaitingItem();
    if (!item) {
        isProcessing = false;
        startButton->setEnabled(true);
        stopButton->setEnabled(false);

        QMessageBox::information(this, tr("Batch Processing Complete"),
                               tr("All items have been processed!"));
        return;
    }

    currentItemIndex = batchQueue->GetItemIndex(item);

    // Mark as processing
    item->MarkAsProcessing();
    batchQueue->NotifyItemStatusChanged(currentItemIndex, BatchItemStatus::Processing);

    // Get encode parameters
    EncodeParameter *encodeParam = item->GetEncodeParameter();
    if (!encodeParam) {
        item->MarkAsFailed("No encode parameters");
        batchQueue->NotifyItemStatusChanged(currentItemIndex, BatchItemStatus::Failed);
        ProcessNextItem();  // Try next item
        return;
    }

    // Create process parameter and add this dialog as observer
    ProcessParameter *processParam = new ProcessParameter();
    processParam->add_observer(this);

    // Start conversion in separate thread
    QString inputPath = item->GetInputPath();
    QString outputPath = item->GetOutputPath();

    QThread *thread = QThread::create([this, inputPath, outputPath, encodeParam, processParam]() {
        bool success = false;

        try {
            Converter converter(processParam, encodeParam);
            converter.set_transcoder("FFMPEG");
            success = converter.convert_format(inputPath.toStdString(), outputPath.toStdString());
        } catch (...) {
            success = false;
        }

        // Remove observer and clean up
        processParam->remove_observer(this);
        delete processParam;

        // Notify on main thread
        QMetaObject::invokeMethod(this, [this, success]() {
            OnConversionFinished(success);
        }, Qt::QueuedConnection);
    });

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void BatchQueueDialog::OnConversionFinished(bool success) {
    if (currentItemIndex >= 0) {
        BatchItem *item = batchQueue->GetItem(currentItemIndex);
        if (item) {
            if (success) {
                item->SetProgress(100.0);
                item->MarkAsFinished();
                batchQueue->NotifyItemStatusChanged(currentItemIndex, BatchItemStatus::Finished);
            } else {
                item->MarkAsFailed("Conversion failed");
                batchQueue->NotifyItemStatusChanged(currentItemIndex, BatchItemStatus::Failed);
            }
        }
    }

    currentItemIndex = -1;

    // Process next item
    ProcessNextItem();
}

void BatchQueueDialog::on_process_update(double progress) {
    if (currentItemIndex >= 0) {
        BatchItem *item = batchQueue->GetItem(currentItemIndex);
        if (item) {
            // Update on main thread
            QMetaObject::invokeMethod(this, [this, progress]() {
                if (currentItemIndex >= 0) {
                    BatchItem *item = batchQueue->GetItem(currentItemIndex);
                    if (item) {
                        item->SetProgress(progress);
                        batchQueue->NotifyItemProgressChanged(currentItemIndex, progress);
                    }
                }
            }, Qt::QueuedConnection);
        }
    }
}

void BatchQueueDialog::on_time_update(double timeRequired) {
    // Not used for now
    Q_UNUSED(timeRequired);
}
