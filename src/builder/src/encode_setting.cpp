#include "../include/encode_setting.h"
#include "ui_encode_setting.h"

/* Receive pointers from widget */
EncodeSetting::EncodeSetting(QWidget *parent, EncodeParameter *encodeParamter)
    : QMainWindow(parent), encodeParameter(encodeParamter),
      ui(new Ui::EncodeSetting) {
    ui->setupUi(this);

    connect(ui->pushButton_cancel, SIGNAL(clicked(bool)), this,
            SLOT(cancel_Pushed()));

    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this,
            SLOT(apply_Pushed()));
}

void EncodeSetting::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QMainWindow::changeEvent(event);
}

bool EncodeSetting::get_Available() { return encodeParameter->get_Available(); }

bool EncodeSetting::get_Encode_Parameter(EncodeParameter *ep) {
    ep = encodeParameter;
    if (ep != NULL) {
        return true;
    }

    return false;
}

void EncodeSetting::cancel_Pushed() {
    // close the sub window
    close();
}

void EncodeSetting::apply_Pushed() {
    /* get the encoder's parameter of video */
    if (!ui->lineEdit_videoCodec->text().toStdString().empty())
        encodeParameter->set_Video_Codec_Name(
            ui->lineEdit_videoCodec->text().toStdString());
    else
        encodeParameter->set_Video_Codec_Name("");
    encodeParameter->set_Video_Bit_Rate(
        ui->lineEdit_videoBitRate->text().toLong());

    if (!ui->lineEdit_audioCodec->text().toStdString().empty())
        encodeParameter->set_Audio_Codec_Name(
            ui->lineEdit_audioCodec->text().toStdString());
    else
        encodeParameter->set_Audio_Codec_Name("");
    encodeParameter->set_Audio_Bit_Rate(
        ui->lineEdit_audioBitRate->text().toLong());
    // close the sub window
    close();
}

EncodeSetting::~EncodeSetting() { delete ui; }
