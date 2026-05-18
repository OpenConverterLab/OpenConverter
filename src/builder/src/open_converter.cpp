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

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCheckBox>
#include <QDebug>
#include <QDialog>
#include <QDir>
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
#include <QPixmap>
#include <QPushButton>
#include <QStatusBar>
#include <QString>
#include <QThread>
#include <QToolButton>
#include <QTranslator>
#include <QUrl>
#include <QVBoxLayout>
#include <QStandardPaths>

#include "../../common/include/encode_parameter.h"
#include "../../common/include/info.h"
#include "../../common/include/process_observer.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include "../include/base_page.h"
#include "../include/batch_queue_dialog.h"
#include "../include/compress_picture_page.h"
#include "../include/create_gif_page.h"
#include "../include/cut_video_page.h"
#include "../include/extract_audio_page.h"
#include "../include/info_view_page.h"
#include "../include/open_converter.h"
#include "../include/placeholder_page.h"
#include "../include/remux_page.h"
#include "../include/shared_data.h"
#include "../include/transcode_page.h"
#include "../include/ai_processing_page.h"
#include "ui_open_converter.h"

#include <iostream>

OpenConverter::OpenConverter(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::OpenConverter) {
    /* init objects */
    info = new Info;
    encodeParameter = new EncodeParameter;
    processParameter = new ProcessParameter;
    converter = new Converter(processParameter, encodeParameter);
    displayResult = new QMessageBox;
    transcoderGroup = new QActionGroup(this);
    languageGroup = new QActionGroup(this);
    QList<QAction*> transcoderActions;

    ui->setupUi(this);
    setAcceptDrops(true);

    // ── File logging ──────────────────────────────────────────────────────
    QString appDataDir = QStandardPaths::writableLocation(
                             QStandardPaths::GenericDataLocation)
                         + "/OpenConverter";
    QDir().mkpath(appDataDir);
    m_settingsPath = appDataDir + "/settings.ini";
    Logger::Instance().SetLogPath((appDataDir + "/openconverter.log").toStdString());

    // ── Restore user settings ────────────────────────────────────────────
    QSettings settings(m_settingsPath, QSettings::IniFormat);
    bool loggingEnabled = settings.value("logging/fileLoggingEnabled", false).toBool();
    ui->action_enableLog->setChecked(loggingEnabled);
    Logger::Instance().SetEnabled(loggingEnabled);

    connect(ui->action_enableLog, &QAction::toggled,
            this, &OpenConverter::SlotLogToggled);

    connect(ui->action_about, &QAction::triggered,
            this, &OpenConverter::SlotAbout);

    // macOS: move Settings and About into the application menu
    ui->action_enableLog->setMenuRole(QAction::NoRole);
    ui->action_about->setMenuRole(QAction::AboutRole);

    // Create Preferences action for macOS app menu
    QAction *prefAction = new QAction(tr("Settings"), this);
    prefAction->setMenuRole(QAction::PreferencesRole);
    connect(prefAction, &QAction::triggered, this, [this]() {
        QDialog *settingsDialog = new QDialog(this);
        settingsDialog->setWindowTitle(tr("Settings"));
        settingsDialog->setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout *layout = new QVBoxLayout(settingsDialog);
        QCheckBox *logCheckBox = new QCheckBox(tr("Enable Log File"), settingsDialog);
        logCheckBox->setChecked(Logger::Instance().IsEnabled());
        layout->addWidget(logCheckBox);

        connect(logCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
            ui->action_enableLog->setChecked(checked);
        });

        layout->addStretch();

        QPushButton *clearButton = new QPushButton(tr("Reset All Settings"), settingsDialog);
        layout->addWidget(clearButton);

        connect(clearButton, &QPushButton::clicked, this, [this, settingsDialog]() {
            QMessageBox::StandardButton reply = QMessageBox::question(
                settingsDialog, tr("Reset Settings"),
                tr("Reset all settings to defaults? The application will restart."),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QSettings settings(m_settingsPath, QSettings::IniFormat);
                settings.clear();
                Logger::Instance().SetEnabled(false);
                ui->action_enableLog->setChecked(false);
                settingsDialog->close();
            }
        });

        settingsDialog->setLayout(layout);
        settingsDialog->exec();
    });
    ui->menuSettings->addAction(prefAction);

    setWindowTitle("OpenConverter");
    setWindowIcon(QIcon(":/OpenConverter-logo.png"));

    // Register this class as an observer for process updates
    processParameter->add_observer(this);

    // Initialize shared data
    sharedData = new SharedData();

    // Restore last browsed directory from settings
    QString lastDir = settings.value("app/lastFilePath").toString();
    if (!lastDir.isEmpty())
        sharedData->SetLastDirectory(lastDir);

    // Initialize batch queue dialog
    batchQueueDialog = nullptr;

#ifdef ENABLE_FFMPEG
    QAction *act_ffmpeg = new QAction(tr("FFMPEG"), this);
    act_ffmpeg->setObjectName("FFMPEG");
    transcoderActions.append(act_ffmpeg);
#endif

#ifdef ENABLE_BMF
    QAction *act_bmf = new QAction(tr("BMF"), this);
    act_bmf->setObjectName("BMF");
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
        QString savedTranscoder = settings.value("app/transcoder").toString();
        bool restored = false;
        if (!savedTranscoder.isEmpty()) {
            for (QAction* action : transcoderActions) {
                if (action->objectName() == savedTranscoder) {
                    action->setChecked(true);
                    converter->set_transcoder(savedTranscoder.toStdString());
                    restored = true;
                    break;
                }
            }
        }
        if (!restored) {
            transcoderActions.first()->setChecked(true);
            converter->set_transcoder(transcoderActions.first()->objectName().toStdString());
        }
    }

    languageGroup->setExclusive(true);
    QList<QAction*> languageActions = ui->menuLanguage->actions();
    for (QAction* action : languageActions) {
        action->setCheckable(true);
        languageGroup->addAction(action);
    }

    // Initialize language - restore from settings or default to English
    m_langPath = ":/";
    QString savedLang = settings.value("app/language", "english").toString();

    for (QAction* action : languageActions) {
        if (action->objectName() == savedLang) {
            action->setChecked(true);
            if (savedLang != "english")
                LoadLanguage(savedLang);
            else
                m_currLang = savedLang;
            break;
        }
    }

    // Initialize navigation button group
    navButtonGroup = new QButtonGroup(this);

    // Setup navigation buttons dynamically
    SetupNavigationButtons();

    // Connect navigation button group
    connect(navButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &OpenConverter::OnNavigationButtonClicked);

    // Initialize pages
    InitializePages();

    // Set first page as active
    if (!pages.isEmpty() && !navButtons.isEmpty()) {
        navButtons.first()->setChecked(true);
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
            QSettings settings(m_settingsPath, QSettings::IniFormat);
            settings.setValue("app/transcoder", action->objectName());
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
        LoadLanguage(action->objectName());
        setWindowIcon(action->icon());
        QSettings settings(m_settingsPath, QSettings::IniFormat);
        settings.setValue("app/language", action->objectName());
    }
}

void switchTranslator(QTranslator &translator, const QString &filename) {
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    QString resourcePath = QString(":/%1").arg(filename);
    qDebug() << "Loading translator from:" << resourcePath;

    if (translator.load(resourcePath)) {
        qDebug() << "Translator loaded successfully!";
        qApp->installTranslator(&translator);
    } else {
        qDebug() << "Failed to load translator!";
    }
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

        // Update navigation labels and buttons
        if (labelCommonSection) {
            labelCommonSection->setText(tr("COMMON"));
        }
        if (labelAdvancedSection) {
            labelAdvancedSection->setText(tr("ADVANCED"));
        }
        if (queueButton) {
            queueButton->setText(tr("📋 Queue"));
            queueButton->setToolTip(tr("View batch processing queue"));
        }

        // Update navigation button texts
        QStringList buttonTexts = {
            tr("Info View"),
            tr("Compress Picture"),
            tr("Extract Audio"),
            tr("Cut Video"),
            tr("Create GIF"),
            tr("Remux"),
            tr("Transcode")
        };
#if defined(ENABLE_BMF) && defined(ENABLE_GUI)
        buttonTexts.append(tr("AI Processing"));
#endif
        for (int i = 0; i < navButtons.size() && i < buttonTexts.size(); ++i) {
            navButtons[i]->setText(buttonTexts[i]);
        }

        // Update language in all pages
        for (BasePage *page : pages) {
            if (page) {
                page->RetranslateUi();
            }
        }
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

void OpenConverter::SetupNavigationButtons() {
    QVBoxLayout *navLayout = ui->navVerticalLayout;

    // Helper lambda to create navigation buttons
    auto createNavButton = [this](const QString &text, int index) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, ui->leftNavWidget);
        btn->setCheckable(true);
        navButtonGroup->addButton(btn, index);
        navButtons.append(btn);
        return btn;
    };

    int pageIndex = 0;

    // COMMON section label
    labelCommonSection = new QLabel(tr("COMMON"), ui->leftNavWidget);
    navLayout->addWidget(labelCommonSection);

    // Common pages - always visible
    navLayout->addWidget(createNavButton(tr("Info View"), pageIndex++));
    navLayout->addWidget(createNavButton(tr("Compress Picture"), pageIndex++));
    navLayout->addWidget(createNavButton(tr("Extract Audio"), pageIndex++));
    navLayout->addWidget(createNavButton(tr("Cut Video"), pageIndex++));
    navLayout->addWidget(createNavButton(tr("Create GIF"), pageIndex++));

    // ADVANCED section label
    labelAdvancedSection = new QLabel(tr("ADVANCED"), ui->leftNavWidget);
    navLayout->addWidget(labelAdvancedSection);

    // Advanced pages
    navLayout->addWidget(createNavButton(tr("Remux"), pageIndex++));
    navLayout->addWidget(createNavButton(tr("Transcode"), pageIndex++));

#if defined(ENABLE_BMF) && defined(ENABLE_GUI)
    // AI Processing page - only when BMF is enabled
    navLayout->addWidget(createNavButton(tr("AI Processing"), pageIndex++));
#endif

    // Add spacer to push queue button to bottom
    navLayout->addStretch();

    // Queue button (not part of navigation group)
    queueButton = new QPushButton(tr("📋 Queue"), ui->leftNavWidget);
    queueButton->setCheckable(false);
    queueButton->setToolTip(tr("View batch processing queue"));
    navLayout->addWidget(queueButton);

    // Connect Queue button
    connect(queueButton, &QPushButton::clicked, this, &OpenConverter::OnQueueButtonClicked);
}

void OpenConverter::InitializePages() {
    // Create pages for each navigation item
    // Common section
    pages.append(new InfoViewPage(this));
    pages.append(new CompressPicturePage(this));
    pages.append(new ExtractAudioPage(this));
    pages.append(new CutVideoPage(this));
    pages.append(new CreateGifPage(this));
    // Advanced section
    pages.append(new RemuxPage(this));
    pages.append(new TranscodePage(this));
#if defined(ENABLE_BMF) && defined(ENABLE_GUI)
    pages.append(new AIProcessingPage(this));
#endif

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
    // Save last browsed directory
    QSettings settings(m_settingsPath, QSettings::IniFormat);
    QString lastDir = sharedData->GetLastDirectory();
    if (!lastDir.isEmpty())
        settings.setValue("app/lastFilePath", lastDir);

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
    delete processParameter;
    delete converter;
    delete displayResult;
    delete sharedData;

    if (batchQueueDialog) {
        delete batchQueueDialog;
    }
}

void OpenConverter::OnQueueButtonClicked() {
    // Create dialog if it doesn't exist
    if (!batchQueueDialog) {
        batchQueueDialog = new BatchQueueDialog(this);
    }

    // Refresh queue and show dialog
    batchQueueDialog->RefreshQueue();
    batchQueueDialog->show();
    batchQueueDialog->raise();
    batchQueueDialog->activateWindow();
}

QString OpenConverter::GetCurrentTranscoderName() const {
    // Get the currently checked transcoder action
    QAction *checkedAction = transcoderGroup->checkedAction();
    if (checkedAction) {
        return checkedAction->objectName();
    }
    // Default to FFMPEG if no transcoder is selected
    return "FFMPEG";
}

void OpenConverter::SlotLogToggled(bool checked) {
    QSettings settings(m_settingsPath, QSettings::IniFormat);
    settings.setValue("logging/fileLoggingEnabled", checked);
    Logger::Instance().SetEnabled(checked);
}

void OpenConverter::SlotAbout() {
    QString ffmpegVer = QString("%1.%2")
        .arg(OC_FFMPEG_VERSION / 10)
        .arg(OC_FFMPEG_VERSION % 10);

    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setWindowTitle(tr("About OpenConverter"));
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(aboutDialog);

    QLabel *iconLabel = new QLabel;
    QPixmap logo(":/OpenConverter-logo.png");
    if (!logo.isNull())
        iconLabel->setPixmap(logo.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    QLabel *textLabel = new QLabel(QString(
        "<h3>OpenConverter</h3>"
        "<table>"
        "<tr><td>Version:</td><td>&nbsp;%1</td></tr>"
        "<tr><td>Commit:</td><td>&nbsp;%2</td></tr>"
        "<tr><td>Qt:</td><td>&nbsp;%3</td></tr>"
        "<tr><td>FFmpeg:</td><td>&nbsp;%4</td></tr>"
        "</table>"
        "<br>"
        "<p>A media converter built on FFmpeg, Qt, and BMF.</p>"
        "<p><a href=\"https://github.com/OpenConverterLab/OpenConverter\">"
        "github.com/OpenConverterLab/OpenConverter</a></p>")
        .arg(OC_VERSION)
        .arg(GIT_COMMIT_HASH)
        .arg(QT_VERSION_STR)
        .arg(ffmpegVer));
    textLabel->setOpenExternalLinks(true);
    textLabel->setTextFormat(Qt::RichText);
    layout->addWidget(textLabel);

    aboutDialog->setLayout(layout);
    aboutDialog->exec();
}

#include "open_converter.moc"
