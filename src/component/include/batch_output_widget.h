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

#ifndef BATCH_OUTPUT_WIDGET_H
#define BATCH_OUTPUT_WIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QString>

/**
 * @brief Widget for batch output configuration
 *
 * Provides options for:
 * 1. Output directory selection (optional)
 * 2. Output suffix (default: "-oc-output")
 * 3. Keep original filename option (only when output directory is different)
 *
 * Output path generation:
 * - If output directory is set:
 *   - Keep original name: /output/dir/filename.ext
 *   - With suffix: /output/dir/filename-oc-output.ext
 * - If output directory is not set (same as input):
 *   - /input/dir/filename-oc-output.ext
 */
class BatchOutputWidget : public QWidget {
    Q_OBJECT

public:
    explicit BatchOutputWidget(QWidget *parent = nullptr);
    ~BatchOutputWidget();

    // Get output configuration
    QString GetOutputDirectory() const;
    QString GetOutputSuffix() const;
    bool IsKeepOriginalName() const;
    bool IsUseOutputDirectory() const;

    // Set output configuration
    void SetOutputDirectory(const QString &dir);
    void SetOutputSuffix(const QString &suffix);
    void SetKeepOriginalName(bool keep);

    // Generate output path for a given input file
    QString GenerateOutputPath(const QString &inputPath, const QString &outputExtension = QString()) const;

    // Retranslate UI when language changes
    void RetranslateUi();

signals:
    void OutputConfigChanged();

private slots:
    void OnBrowseOutputDirClicked();
    void OnUseOutputDirToggled(bool checked);
    void OnSuffixChanged(const QString &text);
    void OnKeepOriginalNameToggled(bool checked);

private:
    void SetupUI();
    void UpdateKeepOriginalNameState();
    void UpdateExampleLabel();

    // UI Components
    QGroupBox *groupBox;
    QCheckBox *useOutputDirCheckBox;
    QLineEdit *outputDirLineEdit;
    QPushButton *browseOutputDirButton;
    QLabel *outputDirLabel;
    QLabel *suffixLabel;
    QLineEdit *suffixLineEdit;
    QCheckBox *keepOriginalNameCheckBox;
    QLabel *exampleLabel;

    QString outputDirectory;
    QString outputSuffix;
    bool keepOriginalName;
};

#endif // BATCH_OUTPUT_WIDGET_H
