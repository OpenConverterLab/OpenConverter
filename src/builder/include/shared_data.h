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

#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <QString>

class SharedData {
public:
    SharedData();
    ~SharedData();

    // Input file path
    QString GetInputFilePath() const;
    void SetInputFilePath(const QString &filePath);

    // Output file path
    QString GetOutputFilePath() const;
    void SetOutputFilePath(const QString &filePath);

    // Generate output path from input path and extension
    // If user has manually set output path, returns that
    // Otherwise, generates: /path/to/input-oc-output.ext
    QString GenerateOutputPath(const QString &extension);

    // Check if output path was manually set by user
    bool IsOutputPathManuallySet() const;

    // Reset output path to auto-generate mode
    void ResetOutputPath();

    // Clear all data
    void Clear();

private:
    QString inputFilePath;
    QString outputFilePath;
    bool outputPathManuallySet;
};

#endif // SHARED_DATA_H
