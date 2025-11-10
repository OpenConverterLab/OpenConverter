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

#ifndef COMPRESS_PICTURE_PAGE_H
#define COMPRESS_PICTURE_PAGE_H

#include "base_page.h"
#include "file_selector_widget.h"
#include "batch_output_widget.h"
#include "batch_mode_helper.h"
#include "resolution_widget.h"
#include "pixel_format_widget.h"
#include "quality_widget.h"
#include "format_selector_widget.h"
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

class Converter;
class EncodeParameter;
class ProcessParameter;

class CompressPicturePage : public BasePage {
    Q_OBJECT

public:
    explicit CompressPicturePage(QWidget *parent = nullptr);
    ~CompressPicturePage();

    QString GetPageTitle() const override;
    void OnPageActivated() override;
    void RetranslateUi() override;

protected:
    void OnOutputPathUpdate() override;

private slots:
    void OnInputFileSelected(const QString &filePath);
    void OnOutputFileSelected(const QString &filePath);
    void OnConvertClicked();
    void OnFormatChanged(const QString &format);

private:
    void SetupUI();
    void UpdateOutputPath();
    EncodeParameter* CreateEncodeParameter();

    // UI Components - Input/Output Section
    QVBoxLayout *mainLayout;
    FileSelectorWidget *inputFileSelector;
    FileSelectorWidget *outputFileSelector;
    BatchOutputWidget *batchOutputWidget;

    // UI Components - Settings Section
    QGroupBox *settingsGroupBox;
    QLabel *formatLabel;
    FormatSelectorWidget *formatWidget;
    QLabel *resolutionLabel;
    ResolutionWidget *resolutionWidget;
    QLabel *pixelFormatLabel;
    PixelFormatWidget *pixelFormatWidget;
    QLabel *qualityLabel;
    QualityWidget *qualityWidget;

    // UI Components - Action Section
    QPushButton *convertButton;

    // Backend
    EncodeParameter *encodeParameter;
    ProcessParameter *processParameter;
    Converter *converter;

    // Batch mode helper
    BatchModeHelper *batchModeHelper;
};

#endif // COMPRESS_PICTURE_PAGE_H
