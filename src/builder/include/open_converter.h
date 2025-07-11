/*
 * Copyright 2024 Jack Lau
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

#ifndef OPEN_CONVERTER_H
#define OPEN_CONVERTER_H

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMetaObject>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QString>
#include <QThread>
#include <QToolButton>
#include <QTranslator>
#include <QUrl>

#include "../../common/include/encode_parameter.h"
#include "../../common/include/info.h"
#include "../../common/include/process_observer.h"
#include "../../common/include/process_parameter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class OpenConverter;
}
QT_END_NAMESPACE

class EncodeSetting;
class Converter;

class OpenConverter : public QMainWindow, public ProcessObserver {
    Q_OBJECT

public:
    explicit OpenConverter(QWidget *parent = nullptr);
    ~OpenConverter();

    // ProcessObserver interface implementation
    void onProcessUpdate(double progress) override;
    void onTimeUpdate(double timeRequired) override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void slotLanguageChanged(QAction *action);
    void slotTranscoderChanged(QAction *action);
    void apply_Pushed();
    void convert_Pushed();
    void encode_Setting_Pushed();

private:
    Ui::OpenConverter *ui;
    QTranslator m_translator;
    QString m_currLang;
    QString m_langPath;
    QString currentInputPath;
    QString currentOutputPath;

    Info *info;
    EncodeParameter *encodeParameter;
    EncodeSetting *encodeSetting;
    ProcessParameter *processParameter;
    Converter *converter;
    QMessageBox *displayResult;

    void loadLanguage(const QString &rLanguage);
    void handle_Converter_Result(bool flag);
    void info_Display(QuickInfo *info);
    QString formatBitrate(int64_t bitsPerSec);
    QString formatFrequency(int64_t hertz);
};

#endif // OPEN_CONVERTER_H
