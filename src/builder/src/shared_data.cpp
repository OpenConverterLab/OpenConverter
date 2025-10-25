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

#include "../include/shared_data.h"
#include <QFileInfo>

SharedData::SharedData() {
    inputFilePath = "";
    outputFilePath = "";
    outputPathManuallySet = false;
}

SharedData::~SharedData() {
}

QString SharedData::GetInputFilePath() const {
    return inputFilePath;
}

void SharedData::SetInputFilePath(const QString &filePath) {
    inputFilePath = filePath;
    // When input path changes, reset output path to auto-generate mode
    if (!outputPathManuallySet) {
        outputFilePath = "";
    }
}

QString SharedData::GetOutputFilePath() const {
    return outputFilePath;
}

void SharedData::SetOutputFilePath(const QString &filePath) {
    outputFilePath = filePath;
    outputPathManuallySet = true;
}

QString SharedData::GenerateOutputPath(const QString &extension) {
    // If user manually set output path, return it
    if (outputPathManuallySet && !outputFilePath.isEmpty()) {
        return outputFilePath;
    }

    // If no input file, return empty
    if (inputFilePath.isEmpty()) {
        return "";
    }

    // Generate output path: /path/to/input-oc-output.ext
    QFileInfo fileInfo(inputFilePath);
    QString baseName = fileInfo.completeBaseName();
    QString dir = fileInfo.absolutePath();

    QString generatedPath = QString("%1/%2-oc-output.%3").arg(dir, baseName, extension);

    // Cache the generated path
    outputFilePath = generatedPath;

    return generatedPath;
}

bool SharedData::IsOutputPathManuallySet() const {
    return outputPathManuallySet;
}

void SharedData::ResetOutputPath() {
    outputFilePath = "";
    outputPathManuallySet = false;
}

void SharedData::Clear() {
    inputFilePath = "";
    outputFilePath = "";
    outputPathManuallySet = false;
}
