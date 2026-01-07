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
#include <QDebug>
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
#include <QPushButton>
#include <QStatusBar>
#include <QString>
#include <QThread>
#include <QToolButton>
#include <QTranslator>
#include <QUrl>
#include <QVBoxLayout>

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
    setWindowTitle("OpenConverter");
    setWindowIcon(QIcon(":/OpenConverter-logo.png"));

    // Register this class as an observer for process updates
    processParameter->add_observer(this);

    // Initialize shared data
    sharedData = new SharedData();

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
        transcoderActions.first()->setChecked(true);
        converter->set_transcoder(transcoderActions.first()->objectName().toStdString());
    }

    languageGroup->setExclusive(true);
    QList<QAction*> languageActions = ui->menuLanguage->actions();
    for (QAction* action : languageActions) {
        action->setCheckable(true);
        languageGroup->addAction(action);
    }

    // Setup Python menu
    pythonGroup = new QActionGroup(this);
    pythonGroup->setExclusive(true);
    QList<QAction*> pythonActions = ui->menuPython->actions();
    for (QAction* action : pythonActions) {
        action->setCheckable(true);
        pythonGroup->addAction(action);
    }

    // Load saved Python setting or default to App Python
    QSettings settings("OpenConverter", "OpenConverter");
    QString savedPython = settings.value("python/mode", "pythonAppSupport").toString();
    customPythonPath = settings.value("python/customPath", "").toString();

    for (QAction* action : pythonActions) {
        if (action->objectName() == savedPython) {
            action->setChecked(true);
            break;
        }
    }

    // Initialize language - default to English (no translation file needed)
    m_currLang = "english";
    m_langPath = ":/";

    // Set the English menu item as checked by default
    for (QAction* action : languageActions) {
        if (action->objectName() == "english") {
            action->setChecked(true);
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

    connect(ui->menuPython, SIGNAL(triggered(QAction *)), this,
            SLOT(SlotPythonChanged(QAction *)));
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

// Called every time, when a menu entry of the Python menu is called
void OpenConverter::SlotPythonChanged(QAction *action) {
    if (!action) return;

    QString pythonMode = action->objectName();
    QSettings settings("OpenConverter", "OpenConverter");

    if (pythonMode == "pythonCustom") {
        // Show file dialog to select site-packages path
        QString dir = QFileDialog::getExistingDirectory(
            this,
            tr("Select Python site-packages Directory"),
            customPythonPath.isEmpty() ? "/opt/homebrew/lib/python3.9/site-packages" : customPythonPath,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

        if (!dir.isEmpty()) {
            customPythonPath = dir;
            settings.setValue("python/mode", pythonMode);
            settings.setValue("python/customPath", customPythonPath);
            ui->statusBar->showMessage(
                tr("Python path set to: %1").arg(customPythonPath));
        } else {
            // User cancelled, revert to previous selection
            QString savedPython = settings.value("python/mode", "pythonAppSupport").toString();
            QList<QAction*> pythonActions = ui->menuPython->actions();
            for (QAction* act : pythonActions) {
                if (act->objectName() == savedPython) {
                    act->setChecked(true);
                    break;
                }
            }
            return;
        }
    } else {
        settings.setValue("python/mode", pythonMode);
        if (pythonMode == "pythonAppSupport") {
            ui->statusBar->showMessage(tr("Using App Python"));
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

QString OpenConverter::GetPythonSitePackagesPath() const {
    QAction *checkedAction = pythonGroup->checkedAction();
    if (checkedAction) {
        QString mode = checkedAction->objectName();
        if (mode == "pythonCustom" && !customPythonPath.isEmpty()) {
            return customPythonPath;
        } else if (mode == "pythonAppSupport") {
            // Python installed in ~/Library/Application Support/OpenConverter/
            QString appSupportPath = QDir::homePath() +
                "/Library/Application Support/OpenConverter/Python.framework/lib/python3.9/site-packages";
            return appSupportPath;
        }
    }
    // Default: Bundled or empty (transcoder will use bundled)
    return QString();
}

QString OpenConverter::GetStoredPythonPath() {
    // Static method that can be called from transcoder_bmf without GUI instance
    QSettings settings("OpenConverter", "OpenConverter");
    QString pythonMode = settings.value("python/mode", "pythonAppSupport").toString();

    if (pythonMode == "pythonCustom") {
        return settings.value("python/customPath", "").toString();
    }
    // Default to App Python (~/Library/Application Support/OpenConverter/)
    QString appSupportPath = QDir::homePath() +
        "/Library/Application Support/OpenConverter/Python.framework/lib/python3.9/site-packages";
    return appSupportPath;
}

#include "open_converter.moc"
