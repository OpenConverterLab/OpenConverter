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
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
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
    void OnBrowseInputClicked();
    void OnBrowseOutputClicked();
    void OnConvertClicked();
    void OnInputFileChanged(const QString &text);
    void OnFormatChanged(int index);

private:
    void SetupUI();
    void UpdateOutputPath();

    // UI Components - Input Section
    QVBoxLayout *mainLayout;
    QGroupBox *inputGroupBox;
    QLineEdit *inputFileLineEdit;
    QPushButton *browseInputButton;

    // UI Components - Settings Section
    QGroupBox *settingsGroupBox;
    QLabel *formatLabel;
    QComboBox *formatComboBox;
    QLabel *widthLabel;
    QSpinBox *widthSpinBox;
    QLabel *heightLabel;
    QSpinBox *heightSpinBox;
    QLabel *pixFmtLabel;
    QComboBox *pixFmtComboBox;
    QLabel *qualityLabel;
    QSpinBox *qualitySpinBox;

    // UI Components - Output Section
    QGroupBox *outputGroupBox;
    QLineEdit *outputFileLineEdit;
    QPushButton *browseOutputButton;
    QPushButton *convertButton;

    // Backend
    EncodeParameter *encodeParameter;
    ProcessParameter *processParameter;
    Converter *converter;
};

#endif // COMPRESS_PICTURE_PAGE_H
