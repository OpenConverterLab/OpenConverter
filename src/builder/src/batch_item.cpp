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

#include "../include/batch_item.h"

BatchItem::BatchItem()
    : status(BatchItemStatus::Waiting),
      encodeParameter(nullptr),
      progress(0.0) {
    createdTime = QDateTime::currentDateTime();
}

BatchItem::BatchItem(const QString &inputPath, const QString &outputPath)
    : inputPath(inputPath),
      outputPath(outputPath),
      status(BatchItemStatus::Waiting),
      encodeParameter(nullptr),
      progress(0.0) {
    createdTime = QDateTime::currentDateTime();
}

BatchItem::~BatchItem() {
    // Note: encodeParameter is managed externally, don't delete here
}

QString BatchItem::GetInputPath() const {
    return inputPath;
}

QString BatchItem::GetOutputPath() const {
    return outputPath;
}

BatchItemStatus BatchItem::GetStatus() const {
    return status;
}

QString BatchItem::GetStatusString() const {
    switch (status) {
        case BatchItemStatus::Waiting:
            return "Waiting";
        case BatchItemStatus::Processing:
            return "Processing";
        case BatchItemStatus::Finished:
            return "Finished";
        case BatchItemStatus::Failed:
            return "Failed";
        default:
            return "Unknown";
    }
}

QString BatchItem::GetErrorMessage() const {
    return errorMessage;
}

EncodeParameter* BatchItem::GetEncodeParameter() const {
    return encodeParameter;
}

QString BatchItem::GetTranscoderName() const {
    return transcoderName.isEmpty() ? "FFMPEG" : transcoderName;
}

QDateTime BatchItem::GetCreatedTime() const {
    return createdTime;
}

QDateTime BatchItem::GetStartedTime() const {
    return startedTime;
}

QDateTime BatchItem::GetFinishedTime() const {
    return finishedTime;
}

double BatchItem::GetProgress() const {
    return progress;
}

void BatchItem::SetInputPath(const QString &path) {
    inputPath = path;
}

void BatchItem::SetOutputPath(const QString &path) {
    outputPath = path;
}

void BatchItem::SetStatus(BatchItemStatus newStatus) {
    status = newStatus;
}

void BatchItem::SetErrorMessage(const QString &message) {
    errorMessage = message;
}

void BatchItem::SetEncodeParameter(EncodeParameter *param) {
    encodeParameter = param;
}

void BatchItem::SetTranscoderName(const QString &name) {
    transcoderName = name;
}

void BatchItem::SetProgress(double newProgress) {
    progress = newProgress;
}

void BatchItem::MarkAsProcessing() {
    status = BatchItemStatus::Processing;
    startedTime = QDateTime::currentDateTime();
    progress = 0.0;
}

void BatchItem::MarkAsFinished() {
    status = BatchItemStatus::Finished;
    finishedTime = QDateTime::currentDateTime();
    progress = 100.0;
}

void BatchItem::MarkAsFailed(const QString &errorMsg) {
    status = BatchItemStatus::Failed;
    finishedTime = QDateTime::currentDateTime();
    errorMessage = errorMsg;
}
