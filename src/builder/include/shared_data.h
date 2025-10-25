/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
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
