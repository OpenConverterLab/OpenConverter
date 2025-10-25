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
