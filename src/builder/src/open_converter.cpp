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
#include "../../engine/include/converter.h"
#include "../include/encode_setting.h"
#include "../include/open_converter.h"
#include "ui_open_converter.h"

#include <iostream>

OpenConverter::OpenConverter(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::OpenConverter) {
    /* init objects */
    info = new Info;
    encodeParameter = new EncodeParameter;
    encodeSetting = new EncodeSetting(nullptr, encodeParameter);
    encodeSetting->setWindowTitle("Encode Setting");
    processParameter = new ProcessParameter;
    converter = new Converter(processParameter, encodeParameter);
    displayResult = new QMessageBox;

    ui->setupUi(this);
    setAcceptDrops(true);
    setWindowTitle("OpenConverter");
    setWindowIcon(QIcon(":/icon/icon.png"));

    ui->progressBar->setValue(0);

    // Register this class as an observer for process updates
    processParameter->add_observer(this);

    connect(ui->toolButton, &QToolButton::clicked, [&]() {
        QString filename = QFileDialog::getOpenFileName();
        ui->lineEdit_inputFile->setText(filename);
    });

    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this,
            SLOT(ApplyPushed()));

    connect(ui->pushButton_convert, SIGNAL(clicked(bool)), this,
            SLOT(ConvertPushed()));

    connect(ui->pushButton_encodeSetting, SIGNAL(clicked(bool)), this,
            SLOT(EncodeSettingPushed()));

    connect(ui->menuLanguage, SIGNAL(triggered(QAction *)), this,
            SLOT(SlotLanguageChanged(QAction *)));

    connect(ui->menuTranscoder, SIGNAL(triggered(QAction *)), this,
            SLOT(SlotTranscoderChanged(QAction *)));

    m_currLang = "english";
    m_langPath = ":/";
}

void OpenConverter::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void OpenConverter::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        const QUrl url = event->mimeData()->urls().first();
        ui->lineEdit_inputFile->setText(url.toLocalFile());
        event->acceptProposedAction();
    }
}

// Called every time, when a menu entry of the transcoder menu is called
void OpenConverter::SlotTranscoderChanged(QAction *action) {
    if (0 != action) {
        std::string transcoderName = action->objectName().toStdString();
        bool isValid = false;
#ifdef ENABLE_FFMPEG
        if (transcoderName == "FFMPEG") {
            converter->set_transcoder(transcoderName);
            isValid = true;
        }
#endif
#ifdef ENABLE_FFTOOL
        if (transcoderName == "FFTOOL") {
            converter->set_transcoder(transcoderName);
            isValid = true;
        }
#endif
#ifdef ENABLE_BMF
        if (transcoderName == "BMF") {
            converter->set_transcoder(transcoderName);
            isValid = true;
        }
#endif
        // If the transcoder name is not valid, log an error
        if (isValid) {
            ui->statusBar->showMessage(
                tr("Current Transcoder changed to %1")
                    .arg(QString::fromStdString(transcoderName)));
        } else {
            std::cout << "Error: Undefined transcoder name - "
                      << transcoderName.c_str() << std::endl;
        }
    }
}

// Called every time, when a menu entry of the language menu is called
void OpenConverter::SlotLanguageChanged(QAction *action) {
    if (0 != action) {
        // load the language dependent on the action content
        LoadLanguage(action->objectName());
        setWindowIcon(action->icon());
    }
}

void switchTranslator(QTranslator &translator, const QString &filename) {
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    //    QString path = QApplication::applicationDirPath();
    //    path.append(":/");
    if (translator.load(QString(":/%1").arg(
            filename))) // Here Path and Filename has to be entered because the
                        // system didn't find the QM Files else
        qApp->installTranslator(&translator);
}

void OpenConverter::LoadLanguage(const QString &rLanguage) {
    if (m_currLang != rLanguage) {
        m_currLang = rLanguage;
        //        QLocale locale = QLocale(m_currLang);
        //        QLocale::setDefault(locale);
        //        QString languageName =
        //        QLocale::languageToString(locale.language());
        switchTranslator(m_translator, QString("lang_%1.qm").arg(rLanguage));
        //        switchTranslator(m_translatorQt,
        //        QString("qt_%1.qm").arg(rLanguage));
        ui->statusBar->showMessage(
            tr("Current Language changed to %1").arg(rLanguage));
    }
}

void OpenConverter::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        // save the current input and output folders
        currentInputPath = ui->lineEdit_inputFile->text();
        currentOutputPath = ui->lineEdit_outputFile->text();

        ui->retranslateUi(this);

        // restore the input and output folders
        ui->lineEdit_inputFile->setText(currentInputPath);
        ui->lineEdit_outputFile->setText(currentOutputPath);

        if (info && info->get_quick_info()) {
            InfoDisplay(info->get_quick_info()); // Convert QuickInfo to string
        }
    }
    QMainWindow::changeEvent(event);
}

void OpenConverter::HandleConverterResult(bool flag) {
    if (flag) {
        displayResult->setText("Convert success!");
        ui->label_timeRequiredResult->setText(QString("%1s").arg(0));
    } else {
        displayResult->setText("Convert failed! Please ensure the file path "
                               "and encode setting is correct");
    }
    displayResult->show();
}

void OpenConverter::on_process_update(double progress) {
    int process = progress;
    ui->progressBar->setValue(process);
    ui->label_processResult->setText(QString("%1%").arg(process));
}

void OpenConverter::on_time_update(double timeRequired) {
    ui->label_timeRequiredResult->setText(
        QString("%1s").arg(QString::number(timeRequired, 'f', 2)));
}

void OpenConverter::EncodeSettingPushed() { encodeSetting->show(); }

void OpenConverter::ApplyPushed() {

    QByteArray ba = ui->lineEdit_inputFile->text().toLocal8Bit();
    char *src = ba.data();
    // get info by Decapsulation
    info->send_info(src);

    // display info on window
    InfoDisplay(info->get_quick_info());
}

void OpenConverter::ConvertPushed() {

    // get the input file path
    QString inputFilePath = ui->lineEdit_inputFile->text();
    // check the input file path
    if (inputFilePath.isEmpty()) {
        displayResult->setText("Please select an input file.");
        displayResult->exec();
        return;
    }
    // get the output file path
    QString outputFilePath = ui->lineEdit_outputFile->text();
    // if the output file path is empty, generate a default output filename
    if (outputFilePath.isEmpty()) {
        QFileInfo fileInfo(inputFilePath);
        outputFilePath = fileInfo.absolutePath() + "/" +
                         fileInfo.completeBaseName() + "-oc-output." +
                         fileInfo.suffix();
        ui->lineEdit_outputFile->setText(outputFilePath);
    }

    // Check if the input file and output file are the same
    if (inputFilePath == outputFilePath) {
        displayResult->setText(
            "The input file can't be the same as the output file!");
        displayResult->exec();
        return;
    }

    // Start conversion in worker thread

    // capture everything you need by value
    auto *thread = QThread::create([=]() {
        bool ok = converter->convert_format(inputFilePath.toStdString(),
                                            outputFilePath.toStdString());
        // When done, marshal back to the GUI thread:
        QMetaObject::invokeMethod(
            this, [this, ok]() { HandleConverterResult(ok); },
            Qt::QueuedConnection);
    });

    // clean up the QThread object once it finishes
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // fire off
    thread->start();
    ;
}

// automatically select kbps/Mbps
QString OpenConverter::FormatBitrate(int64_t bitsPerSec) {
    const double kbps = bitsPerSec / 1000.0;
    if (kbps >= 1000.0) {
        return QString("%1 Mbps").arg(kbps / 1000.0, 0, 'f', 1);
    }
    return QString("%1 kbps").arg(kbps, 0, 'f', 1);
}

// automatically select Hz/kHz/MHz
QString OpenConverter::FormatFrequency(int64_t hertz) {
    const double kHz = hertz / 1000.0;
    if (kHz >= 1000.0) {
        return QString("%1 MHz").arg(kHz / 1000.0, 0, 'f', 2);
    } else if (kHz >= 1.0) {
        return QString("%1 kHz").arg(kHz, 0, 'f', 1);
    }
    return QString("%1 Hz").arg(hertz);
}

void OpenConverter::InfoDisplay(QuickInfo *quickInfo) {
    if (!quickInfo)
        return;

    // video
    ui->label_videoStreamResult->setText(
        QString("%1").arg(quickInfo->videoIdx));
    ui->label_widthResult->setText(QString("%1 px").arg(quickInfo->width));
    ui->label_heightResult->setText(QString("%1 px").arg(quickInfo->height));
    ui->label_colorSpaceResult->setText(
        QString("%1").arg(QString::fromStdString(quickInfo->colorSpace)));
    ui->label_videoCodecResult->setText(
        QString("%1").arg(QString::fromStdString(quickInfo->videoCodec)));
    ui->label_videoBitRateResult->setText(
        FormatBitrate(quickInfo->videoBitRate));
    ui->label_frameRateResult->setText(
        QString("%1 fps").arg(quickInfo->frameRate, 0, 'f', 2));
    // audio
    ui->label_audioStreamResult->setText(
        QString("%1").arg(quickInfo->audioIdx));
    ui->label_audioCodecResult->setText(
        QString("%1").arg(QString::fromStdString(quickInfo->audioCodec)));
    ui->label_audioBitRateResult->setText(
        FormatBitrate(quickInfo->audioBitRate));
    ui->label_channelsResult->setText(QString("%1").arg(quickInfo->channels));
    ui->label_sampleFmtResult->setText(
        QString("%1").arg(QString::fromStdString(quickInfo->sampleFmt)));
    ui->label_sampleRateResult->setText(FormatFrequency(quickInfo->sampleRate));
}

OpenConverter::~OpenConverter() {
    // Remove observer before deleting processParameter
    if (processParameter) {
        processParameter->remove_observer(this);
    }

    delete ui;
    delete info;
    delete encodeParameter;
    delete encodeSetting;
    delete processParameter;
    delete converter;
    delete displayResult;
}

#include "open_converter.moc"
