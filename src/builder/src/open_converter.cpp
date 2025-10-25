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
#include "../include/base_page.h"
#include "../include/compress_picture_page.h"
#include "../include/encode_setting.h"
#include "../include/extract_audio_page.h"
#include "../include/info_view_page.h"
#include "../include/open_converter.h"
#include "../include/placeholder_page.h"
#include "../include/remux_page.h"
#include "../include/shared_data.h"
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
    transcoderGroup = new QActionGroup(this);
    languageGroup = new QActionGroup(this);
    QList<QAction*> transcoderActions;

    ui->setupUi(this);
    setAcceptDrops(true);
    setWindowTitle("OpenConverter");
    setWindowIcon(QIcon(":/icon/icon.png"));

    // Register this class as an observer for process updates
    processParameter->add_observer(this);

    // Initialize shared data
    sharedData = new SharedData();

#ifdef ENABLE_FFMPEG
    QAction *act_ffmpeg = new QAction(tr("FFMPEG"), this);
    act_ffmpeg->setObjectName("FFMPEG");
    transcoderActions.append(act_ffmpeg);
#endif

#ifdef ENABLE_BMF
    QAction *act_bmf = new QAction(tr("BMF"), this);
    transcoderActions.append(act_bmf);
#endif

#ifdef ENABLE_FFTOOL
    QAction *act_fftool = new QAction(tr("FFTOOL"), this);
    act_fftool->setObjectName("FFTOOL");
    transcoderActions.append(act_fftool);
#endif

    for (QAction* a : qAsConst(transcoderActions)) {
        if (a) ui->menuTranscoder->addAction(a);
    }


    transcoderGroup->setExclusive(true);
    transcoderActions = ui->menuTranscoder->actions();
    for (QAction* action : transcoderActions) {
        action->setCheckable(true);
        transcoderGroup->addAction(action);
    }

    if (!transcoderActions.isEmpty()) {
        transcoderActions.first()->setChecked(true);
        converter->set_transcoder(transcoderActions.first()->objectName().toStdString());
    }

    languageGroup->setExclusive(true);
    QList<QAction*> languageActions = ui->menuLanguage->actions();
    for (QAction* action : languageActions) {
        action->setCheckable(true);
        languageGroup->addAction(action);
    }

    m_currLang = "english";
    m_langPath = ":/";
    for (QAction* action : languageActions) {
        if (action->objectName() == m_currLang) {
            action->setChecked(true);
            break;
        }
    }

    // Initialize navigation button group
    navButtonGroup = new QButtonGroup(this);
    navButtonGroup->addButton(ui->btnInfoView, 0);
    navButtonGroup->addButton(ui->btnCompressPicture, 1);
    navButtonGroup->addButton(ui->btnExtractAudio, 2);
    navButtonGroup->addButton(ui->btnCutVideo, 3);
    navButtonGroup->addButton(ui->btnRemux, 4);
    navButtonGroup->addButton(ui->btnTranscode, 5);

    // Connect navigation button group
    connect(navButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &OpenConverter::OnNavigationButtonClicked);

    // Initialize pages
    InitializePages();

    // Set first page as active
    if (!pages.isEmpty()) {
        ui->btnInfoView->setChecked(true);
        SwitchToPage(0);
    }

    connect(ui->menuLanguage, SIGNAL(triggered(QAction *)), this,
            SLOT(SlotLanguageChanged(QAction *)));

    connect(ui->menuTranscoder, SIGNAL(triggered(QAction *)), this,
            SLOT(SlotTranscoderChanged(QAction *)));
}

void OpenConverter::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void OpenConverter::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        const QUrl url = event->mimeData()->urls().first();
        QString filePath = url.toLocalFile();

        // Get current page and handle file drop
        int currentIndex = ui->stackedWidget->currentIndex();
        if (currentIndex >= 0 && currentIndex < pages.size()) {
            // If it's the InfoViewPage, handle the drop
            InfoViewPage *infoPage = qobject_cast<InfoViewPage *>(pages[currentIndex]);
            if (infoPage) {
                infoPage->HandleFileDrop(filePath);
            }
        }

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
        ui->retranslateUi(this);
        // TODO: Update language in pages
    }
    QMainWindow::changeEvent(event);
}

void OpenConverter::HandleConverterResult(bool flag) {
    if (flag) {
        displayResult->setText("Convert success!");
    } else {
        displayResult->setText("Convert failed! Please ensure the file path "
                               "and encode setting is correct");
    }
    displayResult->show();
}

void OpenConverter::on_process_update(double progress) {
    // This can be implemented later for progress tracking in pages
}

void OpenConverter::on_time_update(double timeRequired) {
    // This can be implemented later for time tracking in pages
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
    // This can be implemented later for displaying info in pages
}

void OpenConverter::InitializePages() {
    // Create pages for each navigation item
    // Common section
    pages.append(new InfoViewPage(this));
    pages.append(new CompressPicturePage(this));
    pages.append(new ExtractAudioPage(this));
    pages.append(new PlaceholderPage("Cut Video", this));
    // Advanced section
    pages.append(new RemuxPage(this));
    pages.append(new PlaceholderPage("Transcode", this));

    // Add all pages to the stacked widget
    for (BasePage *page : pages) {
        ui->stackedWidget->addWidget(page);
    }
}

void OpenConverter::SwitchToPage(int pageIndex) {
    if (pageIndex < 0 || pageIndex >= pages.size()) {
        return;
    }

    // Deactivate current page
    int currentIndex = ui->stackedWidget->currentIndex();
    if (currentIndex >= 0 && currentIndex < pages.size()) {
        pages[currentIndex]->OnPageDeactivated();
    }

    // Switch to new page
    ui->stackedWidget->setCurrentIndex(pageIndex);
    pages[pageIndex]->OnPageActivated();

    // Update window title
    setWindowTitle(QString("OpenConverter - %1").arg(pages[pageIndex]->GetPageTitle()));
}

SharedData* OpenConverter::GetSharedData() const {
    return sharedData;
}

void OpenConverter::OnNavigationButtonClicked(int pageIndex) {
    SwitchToPage(pageIndex);
}

OpenConverter::~OpenConverter() {
    // Remove observer before deleting processParameter
    if (processParameter) {
        processParameter->remove_observer(this);
    }

    // Clean up pages
    qDeleteAll(pages);
    pages.clear();

    delete ui;
    delete info;
    delete encodeParameter;
    delete encodeSetting;
    delete processParameter;
    delete converter;
    delete displayResult;
    delete sharedData;
}

#include "open_converter.moc"
