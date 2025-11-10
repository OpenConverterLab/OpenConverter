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

#ifndef BATCH_ITEM_H
#define BATCH_ITEM_H

#include <QString>
#include <QDateTime>
#include "encode_parameter.h"

/**
 * @brief Status of a batch item
 */
enum class BatchItemStatus {
    Waiting,      // Waiting to be processed
    Processing,   // Currently being processed
    Finished,     // Successfully completed
    Failed        // Failed with error
};

/**
 * @brief Represents a single item in the batch processing queue
 *
 * Each BatchItem contains:
 * - Input file path
 * - Output file path
 * - Encoding parameters (codec, bitrate, resolution, etc.)
 * - Processing status (waiting, processing, finished, failed)
 * - Error message (if failed)
 * - Timestamps for tracking
 */
class BatchItem {
public:
    BatchItem();
    BatchItem(const QString &inputPath, const QString &outputPath);
    ~BatchItem();

    // Getters
    QString GetInputPath() const;
    QString GetOutputPath() const;
    BatchItemStatus GetStatus() const;
    QString GetStatusString() const;
    QString GetErrorMessage() const;
    EncodeParameter* GetEncodeParameter() const;
    QString GetTranscoderName() const;
    QDateTime GetCreatedTime() const;
    QDateTime GetStartedTime() const;
    QDateTime GetFinishedTime() const;
    double GetProgress() const;

    // Setters
    void SetInputPath(const QString &path);
    void SetOutputPath(const QString &path);
    void SetStatus(BatchItemStatus status);
    void SetErrorMessage(const QString &message);
    void SetEncodeParameter(EncodeParameter *param);
    void SetTranscoderName(const QString &name);
    void SetProgress(double progress);

    // Status management
    void MarkAsProcessing();
    void MarkAsFinished();
    void MarkAsFailed(const QString &errorMessage);

private:
    QString inputPath;
    QString outputPath;
    BatchItemStatus status;
    QString errorMessage;
    EncodeParameter *encodeParameter;
    QString transcoderName;  // Transcoder to use (e.g., "FFMPEG", "BMF", "FFTOOL")
    QDateTime createdTime;
    QDateTime startedTime;
    QDateTime finishedTime;
    double progress;  // 0.0 to 100.0
};

#endif // BATCH_ITEM_H
