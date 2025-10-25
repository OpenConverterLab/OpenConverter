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
