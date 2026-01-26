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

#ifndef CREATE_GIF_PAGE_H
#define CREATE_GIF_PAGE_H

#include "base_page.h"
#include "file_selector_widget.h"
#include "batch_output_widget.h"
#include "batch_mode_helper.h"
#include "resolution_widget.h"
#include "progress_widget.h"
#include "converter_runner.h"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

class CreateGifPage : public BasePage {
    Q_OBJECT

public:
    explicit CreateGifPage(QWidget *parent = nullptr);
    ~CreateGifPage();

    QString GetPageTitle() const override;
    void OnPageActivated() override;
    void RetranslateUi() override;

protected:
    void OnOutputPathUpdate() override;

private slots:
    void OnInputFileSelected(const QString &filePath);
    void OnOutputFileSelected(const QString &filePath);
    void OnConvertClicked();
    void OnConvertFinished(bool success);
    void OnFpsChanged(int value);

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
    QLabel *resolutionLabel;
    ResolutionWidget *resolutionWidget;
    QLabel *fpsLabel;
    QSpinBox *fpsSpinBox;
    QLabel *qualityLabel;
    QSpinBox *qualitySpinBox;

    // UI Components - Action Section
    QPushButton *convertButton;
    ProgressWidget *progressWidget;

    // Backend
    ConverterRunner *converterRunner;

    // Batch mode helper
    BatchModeHelper *batchModeHelper;
};

#endif // CREATE_GIF_PAGE_H
