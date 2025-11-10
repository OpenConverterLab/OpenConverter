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

#include "../include/batch_queue.h"

BatchQueue *BatchQueue::instance = nullptr;
QMutex BatchQueue::instanceMutex;

BatchQueue::BatchQueue() : QObject(nullptr) {
}

BatchQueue::~BatchQueue() {
    Clear();
}

BatchQueue* BatchQueue::Instance() {
    if (instance == nullptr) {
        QMutexLocker locker(&instanceMutex);
        if (instance == nullptr) {
            instance = new BatchQueue();
        }
    }
    return instance;
}

void BatchQueue::AddItem(BatchItem *item) {
    if (!item) return;

    QMutexLocker locker(&queueMutex);
    items.append(item);
    int index = items.size() - 1;
    emit ItemAdded(index);
}

void BatchQueue::AddItems(const QList<BatchItem*> &newItems) {
    if (newItems.isEmpty()) return;

    QMutexLocker locker(&queueMutex);
    for (BatchItem *item : newItems) {
        if (item) {
            items.append(item);
            int index = items.size() - 1;
            emit ItemAdded(index);
        }
    }
}

void BatchQueue::RemoveItem(int index) {
    QMutexLocker locker(&queueMutex);
    if (index >= 0 && index < items.size()) {
        BatchItem *item = items.takeAt(index);
        delete item;
        emit ItemRemoved(index);
    }
}

void BatchQueue::Clear() {
    QMutexLocker locker(&queueMutex);
    qDeleteAll(items);
    items.clear();
    emit QueueCleared();
}

int BatchQueue::GetCount() const {
    QMutexLocker locker(&queueMutex);
    return items.size();
}

int BatchQueue::GetWaitingCount() const {
    QMutexLocker locker(&queueMutex);
    int count = 0;
    for (const BatchItem *item : items) {
        if (item->GetStatus() == BatchItemStatus::Waiting) {
            count++;
        }
    }
    return count;
}

int BatchQueue::GetProcessingCount() const {
    QMutexLocker locker(&queueMutex);
    int count = 0;
    for (const BatchItem *item : items) {
        if (item->GetStatus() == BatchItemStatus::Processing) {
            count++;
        }
    }
    return count;
}

int BatchQueue::GetFinishedCount() const {
    QMutexLocker locker(&queueMutex);
    int count = 0;
    for (const BatchItem *item : items) {
        if (item->GetStatus() == BatchItemStatus::Finished) {
            count++;
        }
    }
    return count;
}

int BatchQueue::GetFailedCount() const {
    QMutexLocker locker(&queueMutex);
    int count = 0;
    for (const BatchItem *item : items) {
        if (item->GetStatus() == BatchItemStatus::Failed) {
            count++;
        }
    }
    return count;
}

BatchItem* BatchQueue::GetItem(int index) const {
    QMutexLocker locker(&queueMutex);
    if (index >= 0 && index < items.size()) {
        return items.at(index);
    }
    return nullptr;
}

QList<BatchItem*> BatchQueue::GetAllItems() const {
    QMutexLocker locker(&queueMutex);
    return items;
}

BatchItem* BatchQueue::GetNextWaitingItem() const {
    QMutexLocker locker(&queueMutex);
    for (BatchItem *item : items) {
        if (item->GetStatus() == BatchItemStatus::Waiting) {
            return item;
        }
    }
    return nullptr;
}

bool BatchQueue::IsEmpty() const {
    QMutexLocker locker(&queueMutex);
    return items.isEmpty();
}

bool BatchQueue::HasWaitingItems() const {
    return GetWaitingCount() > 0;
}

bool BatchQueue::IsProcessing() const {
    return GetProcessingCount() > 0;
}

int BatchQueue::GetItemIndex(BatchItem *item) const {
    QMutexLocker locker(&queueMutex);
    return items.indexOf(item);
}

void BatchQueue::NotifyItemStatusChanged(int index, BatchItemStatus status) {
    emit ItemStatusChanged(index, status);
}

void BatchQueue::NotifyItemProgressChanged(int index, double progress) {
    emit ItemProgressChanged(index, progress);
}
