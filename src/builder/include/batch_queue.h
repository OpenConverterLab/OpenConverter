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

#ifndef BATCH_QUEUE_H
#define BATCH_QUEUE_H

#include <QObject>
#include <QList>
#include <QMutex>
#include "batch_item.h"

/**
 * @brief Singleton class to manage batch processing queue
 *
 * The BatchQueue manages a list of BatchItems to be processed.
 * It provides thread-safe access to the queue and emits signals
 * when items are added, removed, or their status changes.
 *
 * Usage:
 *   BatchQueue *queue = BatchQueue::Instance();
 *   queue->AddItem(item);
 *   queue->ProcessNext();
 */
class BatchQueue : public QObject {
    Q_OBJECT

public:
    // Singleton access
    static BatchQueue* Instance();

    // Queue management
    void AddItem(BatchItem *item);
    void AddItems(const QList<BatchItem*> &items);
    void RemoveItem(int index);
    void Clear();

    // Queue access
    int GetCount() const;
    int GetWaitingCount() const;
    int GetProcessingCount() const;
    int GetFinishedCount() const;
    int GetFailedCount() const;
    BatchItem* GetItem(int index) const;
    QList<BatchItem*> GetAllItems() const;
    BatchItem* GetNextWaitingItem() const;

    // Queue state
    bool IsEmpty() const;
    bool HasWaitingItems() const;
    bool IsProcessing() const;

    // Helper methods
    int GetItemIndex(BatchItem *item) const;
    void NotifyItemStatusChanged(int index, BatchItemStatus status);
    void NotifyItemProgressChanged(int index, double progress);

signals:
    void ItemAdded(int index);
    void ItemRemoved(int index);
    void ItemStatusChanged(int index, BatchItemStatus status);
    void ItemProgressChanged(int index, double progress);
    void QueueCleared();
    void AllItemsFinished();

private:
    BatchQueue();  // Private constructor for singleton
    ~BatchQueue();
    BatchQueue(const BatchQueue&) = delete;
    BatchQueue& operator=(const BatchQueue&) = delete;

    static BatchQueue *instance;
    static QMutex instanceMutex;

    QList<BatchItem*> items;
    mutable QMutex queueMutex;
};

#endif // BATCH_QUEUE_H
