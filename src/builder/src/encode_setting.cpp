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

#include "../include/encode_setting.h"
#include "ui_encode_setting.h"

/* Receive pointers from widget */
EncodeSetting::EncodeSetting(QWidget *parent, EncodeParameter *encodeParamter)
    : QMainWindow(parent), encodeParameter(encodeParamter),
      ui(new Ui::EncodeSetting) {
    ui->setupUi(this);

    connect(ui->pushButton_cancel, SIGNAL(clicked(bool)), this,
            SLOT(CancelPushed()));

    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this,
            SLOT(ApplyPushed()));
}

void EncodeSetting::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QMainWindow::changeEvent(event);
}

bool EncodeSetting::GetAvailable() { return encodeParameter->get_available(); }

bool EncodeSetting::GetEncodeParameter(EncodeParameter *ep) {
    ep = encodeParameter;
    if (ep != NULL) {
        return true;
    }

    return false;
}

void EncodeSetting::CancelPushed() {
    // close the sub window
    close();
}

void EncodeSetting::ApplyPushed() {
    /* get the encoder's parameter of video */
    if (!ui->lineEdit_videoCodec->text().toStdString().empty())
        encodeParameter->set_video_codec_name(
            ui->lineEdit_videoCodec->text().toStdString());
    else
        encodeParameter->set_video_codec_name("");
    encodeParameter->set_video_bit_rate(
        ui->lineEdit_videoBitRate->text().toLong());

    if (!ui->lineEdit_audioCodec->text().toStdString().empty())
        encodeParameter->set_audio_codec_name(
            ui->lineEdit_audioCodec->text().toStdString());
    else
        encodeParameter->set_audio_codec_name("");
    encodeParameter->set_audio_bit_rate(
        ui->lineEdit_audioBitRate->text().toLong());
    // close the sub window
    close();
}

EncodeSetting::~EncodeSetting() { delete ui; }
