/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License a
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../include/cut_video_page.h"
#include "../include/open_converter.h"
#include "../include/shared_data.h"
#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

CutVideoPage::CutVideoPage(QWidget *parent)
    : BasePage(parent), videoDuration(0), startTime(0), endTime(0), isSliderPressed(false) {
    SetupUI();
    connect(this, &CutVideoPage::CutComplete, this, &CutVideoPage::OnCutFinished);
}

CutVideoPage::~CutVideoPage() {
    if (videoPlayer) {
        videoPlayer->Stop();
    }
}

void CutVideoPage::OnPageActivated() {
    BasePage::OnPageActivated();
    HandleSharedDataUpdate(inputFileLineEdit, outputFileLineEdit, "mp4");
}

void CutVideoPage::OnInputFileChanged(const QString &newPath) {
    LoadVideo(newPath);
}

void CutVideoPage::OnOutputPathUpdate() {
    UpdateOutputPath();
}

void CutVideoPage::OnPageDeactivated() {
    BasePage::OnPageDeactivated();
    if (videoPlayer) {
        videoPlayer->Pause();
    }
}

void CutVideoPage::SetupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Input File Section
    inputGroupBox = new QGroupBox(tr("Input File"), this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroupBox);

    inputFileLineEdit = new QLineEdit(inputGroupBox);
    inputFileLineEdit->setPlaceholderText(tr("Select a video file..."));
    inputFileLineEdit->setReadOnly(true);

    browseInputButton = new QPushButton(tr("Browse..."), inputGroupBox);
    connect(browseInputButton, &QPushButton::clicked, this, &CutVideoPage::OnBrowseInputClicked);

    inputLayout->addWidget(inputFileLineEdit);
    inputLayout->addWidget(browseInputButton);

    mainLayout->addWidget(inputGroupBox);

    // Media Duration Section
    durationGroupBox = new QGroupBox(tr("Media Duration"), this);
    QHBoxLayout *durationLayout = new QHBoxLayout(durationGroupBox);

    totalDurationLabel = new QLabel(tr("Total Duration:"), durationGroupBox);
    totalDurationLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    totalDurationValueLabel = new QLabel("00:00:00", durationGroupBox);
    totalDurationValueLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    durationLayout->addWidget(totalDurationLabel);
    durationLayout->addWidget(totalDurationValueLabel);
    durationLayout->addStretch();

    mainLayout->addWidget(durationGroupBox);

    // Video Player Section
    playerGroupBox = new QGroupBox(tr("Video Player"), this);
    QVBoxLayout *playerLayout = new QVBoxLayout(playerGroupBox);

    // Simple FFmpeg-based video player
    videoPlayer = new SimpleVideoPlayer(playerGroupBox);
    videoPlayer->setMinimumHeight(300);
    connect(videoPlayer, &SimpleVideoPlayer::PositionChanged, this, &CutVideoPage::OnVideoPlayerPositionChanged);
    connect(videoPlayer, &SimpleVideoPlayer::DurationChanged, this, &CutVideoPage::OnVideoPlayerDurationChanged);

    playerLayout->addWidget(videoPlayer);

    // Player controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    playPauseButton = new QPushButton(tr("Play"), playerGroupBox);
    playPauseButton->setEnabled(false);
    connect(playPauseButton, &QPushButton::clicked, this, &CutVideoPage::OnPlayPauseClicked);

    currentTimeLabel = new QLabel("00:00:00", playerGroupBox);
    currentTimeLabel->setMinimumWidth(70);

    timelineSlider = new QSlider(Qt::Horizontal, playerGroupBox);
    timelineSlider->setRange(0, 0);
    timelineSlider->setEnabled(false);
    connect(timelineSlider, &QSlider::sliderPressed, this, &CutVideoPage::OnTimelineSliderPressed);
    connect(timelineSlider, &QSlider::sliderReleased, this, &CutVideoPage::OnTimelineSliderReleased);
    connect(timelineSlider, &QSlider::sliderMoved, this, &CutVideoPage::OnTimelineSliderMoved);

    endTimeDisplayLabel = new QLabel("00:00:00", playerGroupBox);
    endTimeDisplayLabel->setMinimumWidth(70);
    endTimeDisplayLabel->setAlignment(Qt::AlignRight);

    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(currentTimeLabel);
    controlsLayout->addWidget(timelineSlider);
    controlsLayout->addWidget(endTimeDisplayLabel);

    playerLayout->addLayout(controlsLayout);
    mainLayout->addWidget(playerGroupBox);

    // Time Selection Section
    timeSelectionGroupBox = new QGroupBox(tr("Time Selection"), this);
    QGridLayout *timeSelectionLayout = new QGridLayout(timeSelectionGroupBox);
    timeSelectionLayout->setSpacing(10);

    // Start time
    startTimeLabel = new QLabel(tr("Start Time:"), timeSelectionGroupBox);
    startTimeEdit = new QTimeEdit(QTime(0, 0, 0), timeSelectionGroupBox);
    startTimeEdit->setDisplayFormat("HH:mm:ss");
    connect(startTimeEdit, &QTimeEdit::timeChanged, this, &CutVideoPage::OnStartTimeChanged);

    setStartButton = new QPushButton(tr("Set from Player"), timeSelectionGroupBox);
    setStartButton->setEnabled(false);
    connect(setStartButton, &QPushButton::clicked, this, &CutVideoPage::OnSetStartClicked);

    // End time
    endTimeLabel = new QLabel(tr("End Time:"), timeSelectionGroupBox);
    endTimeEdit = new QTimeEdit(QTime(0, 0, 0), timeSelectionGroupBox);
    endTimeEdit->setDisplayFormat("HH:mm:ss");
    connect(endTimeEdit, &QTimeEdit::timeChanged, this, &CutVideoPage::OnEndTimeChanged);

    setEndButton = new QPushButton(tr("Set from Player"), timeSelectionGroupBox);
    setEndButton->setEnabled(false);
    connect(setEndButton, &QPushButton::clicked, this, &CutVideoPage::OnSetEndClicked);

    // Duration
    cutDurationLabel = new QLabel(tr("Cut Duration:"), timeSelectionGroupBox);
    cutDurationLabel->setStyleSheet("font-weight: bold;");

    cutDurationValueLabel = new QLabel("00:00:00", timeSelectionGroupBox);
    cutDurationValueLabel->setStyleSheet("font-weight: bold;");

    timeSelectionLayout->addWidget(startTimeLabel, 0, 0);
    timeSelectionLayout->addWidget(startTimeEdit, 0, 1);
    timeSelectionLayout->addWidget(setStartButton, 0, 2);
    timeSelectionLayout->addWidget(endTimeLabel, 1, 0);
    timeSelectionLayout->addWidget(endTimeEdit, 1, 1);
    timeSelectionLayout->addWidget(setEndButton, 1, 2);
    timeSelectionLayout->addWidget(cutDurationLabel, 2, 0);
    timeSelectionLayout->addWidget(cutDurationValueLabel, 2, 1, 1, 2);

    mainLayout->addWidget(timeSelectionGroupBox);

    // Progress Section
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);

    progressLabel = new QLabel("", this);
    progressLabel->setVisible(false);

    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(progressLabel);

    // Output File Section
    outputGroupBox = new QGroupBox(tr("Output File"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroupBox);

    QHBoxLayout *outputPathLayout = new QHBoxLayout();
    outputFileLineEdit = new QLineEdit(outputGroupBox);
    outputFileLineEdit->setPlaceholderText(tr("Output file path..."));

    browseOutputButton = new QPushButton(tr("Browse..."), outputGroupBox);
    connect(browseOutputButton, &QPushButton::clicked, this, &CutVideoPage::OnBrowseOutputClicked);

    outputPathLayout->addWidget(outputFileLineEdit);
    outputPathLayout->addWidget(browseOutputButton);

    cutButton = new QPushButton(tr("Cut Video"), outputGroupBox);
    cutButton->setEnabled(false);
    cutButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; font-weight: bold; }");
    connect(cutButton, &QPushButton::clicked, this, &CutVideoPage::OnCutClicked);

    outputLayout->addLayout(outputPathLayout);
    outputLayout->addWidget(cutButton);

    mainLayout->addWidget(outputGroupBox);

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void CutVideoPage::OnBrowseInputClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Video File"),
        "",
        tr("Video Files (*.mp4 *.avi *.mkv *.mov *.flv *.wmv *.webm *.ts *.m4v);;All Files (*.*)")
    );

    if (!fileName.isEmpty()) {
        inputFileLineEdit->setText(fileName);

        // Update shared input file path
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            mainWindow->GetSharedData()->SetInputFilePath(fileName);
        }

        LoadVideo(fileName);
        UpdateOutputPath();
    }
}

void CutVideoPage::OnBrowseOutputClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Cut Video As"),
        "",
        tr("Video Files (*.mp4 *.avi *.mkv *.mov);;All Files (*.*)")
    );

    if (!fileName.isEmpty()) {
        outputFileLineEdit->setText(fileName);

        // Mark output path as manually se
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            mainWindow->GetSharedData()->SetOutputFilePath(fileName);
        }
    }
}

void CutVideoPage::UpdateOutputPath() {
    QString inputPath = inputFileLineEdit->text();
    if (!inputPath.isEmpty()) {
        OpenConverter *mainWindow = qobject_cast<OpenConverter *>(window());
        if (mainWindow && mainWindow->GetSharedData()) {
            QString outputPath = mainWindow->GetSharedData()->GenerateOutputPath("mp4");
            outputFileLineEdit->setText(outputPath);
            cutButton->setEnabled(videoDuration > 0);
        }
    }
}

void CutVideoPage::LoadVideo(const QString &filePath) {
    if (filePath.isEmpty()) {
        return;
    }

    // Get duration quickly using FFmpeg (limit analysis to avoid blocking)
    AVFormatContext *fmtCtx = nullptr;
    int ret = avformat_open_input(&fmtCtx, filePath.toStdString().c_str(), nullptr, nullptr);
    if (ret >= 0) {
        // Limit stream analysis time to avoid blocking UI
        fmtCtx->max_analyze_duration = 5 * AV_TIME_BASE; // 5 seconds max
        ret = avformat_find_stream_info(fmtCtx, nullptr);
        if (ret >= 0 && fmtCtx->duration != AV_NOPTS_VALUE) {
            // Duration is in AV_TIME_BASE units (microseconds)
            qint64 durationMs = (fmtCtx->duration * 1000) / AV_TIME_BASE;
            videoDuration = durationMs;
            timelineSlider->setRange(0, durationMs);
            endTimeDisplayLabel->setText(FormatTime(durationMs));

            // Set default end time to video duration
            endTime = durationMs;
            int hours = (durationMs / 1000) / 3600;
            int minutes = ((durationMs / 1000) % 3600) / 60;
            int seconds = (durationMs / 1000) % 60;
            endTimeEdit->setTime(QTime(hours, minutes, seconds));

            UpdateDurationLabel();
        }
        avformat_close_input(&fmtCtx);
    }

    // Load video in player (deferred to avoid blocking)
    // The first frame decode is now deferred via QTimer::singleSho
    if (videoPlayer->LoadVideo(filePath)) {
        // Video loaded successfully
        playPauseButton->setEnabled(true);
        setStartButton->setEnabled(true);
        setEndButton->setEnabled(true);
        timelineSlider->setEnabled(true);
        cutButton->setEnabled(true);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load video file."));
    }
}

void CutVideoPage::OnPlayPauseClicked() {
    if (videoPlayer->IsPlaying()) {
        videoPlayer->Pause();
        playPauseButton->setText(tr("Play"));
    } else {
        videoPlayer->Play();
        playPauseButton->setText(tr("Pause"));
    }
}

void CutVideoPage::OnVideoPlayerPositionChanged(qint64 position) {
    if (!isSliderPressed) {
        timelineSlider->setValue(position);
    }
    currentTimeLabel->setText(FormatTime(position));
}

void CutVideoPage::OnVideoPlayerDurationChanged(qint64 duration) {
    videoDuration = duration;
    timelineSlider->setRange(0, duration);
    endTimeDisplayLabel->setText(FormatTime(duration));

    // Set default end time to video duration
    if (endTime == 0) {
        endTime = duration;
        int hours = (duration / 1000) / 3600;
        int minutes = ((duration / 1000) % 3600) / 60;
        int seconds = (duration / 1000) % 60;
        endTimeEdit->setTime(QTime(hours, minutes, seconds));
    }

    UpdateDurationLabel();
}

void CutVideoPage::OnTimelineSliderPressed() {
    isSliderPressed = true;
}

void CutVideoPage::OnTimelineSliderReleased() {
    isSliderPressed = false;
    videoPlayer->Seek(timelineSlider->value());
}

void CutVideoPage::OnTimelineSliderMoved(int position) {
    currentTimeLabel->setText(FormatTime(position));
}

void CutVideoPage::OnStartTimeChanged(const QTime &time) {
    startTime = (time.hour() * 3600 + time.minute() * 60 + time.second()) * 1000;
    UpdateDurationLabel();
}

void CutVideoPage::OnEndTimeChanged(const QTime &time) {
    endTime = (time.hour() * 3600 + time.minute() * 60 + time.second()) * 1000;
    UpdateDurationLabel();
}

void CutVideoPage::OnSetStartClicked() {
    qint64 currentPos = videoPlayer->GetPosition();
    int hours = (currentPos / 1000) / 3600;
    int minutes = ((currentPos / 1000) % 3600) / 60;
    int seconds = (currentPos / 1000) % 60;
    startTimeEdit->setTime(QTime(hours, minutes, seconds));
}

void CutVideoPage::OnSetEndClicked() {
    qint64 currentPos = videoPlayer->GetPosition();
    int hours = (currentPos / 1000) / 3600;
    int minutes = ((currentPos / 1000) % 3600) / 60;
    int seconds = (currentPos / 1000) % 60;
    endTimeEdit->setTime(QTime(hours, minutes, seconds));
}

void CutVideoPage::UpdateDurationLabel() {
    // Update total duration value
    totalDurationValueLabel->setText(FormatTime(videoDuration));

    // Update cut duration value
    if (endTime > startTime) {
        qint64 duration = endTime - startTime;
        cutDurationValueLabel->setText(FormatTime(duration));
    } else {
        cutDurationValueLabel->setText(tr("Invalid (end time must be after start time)"));
    }
}

QString CutVideoPage::FormatTime(qint64 milliseconds) {
    int seconds = (milliseconds / 1000) % 60;
    int minutes = ((milliseconds / 1000) / 60) % 60;
    int hours = (milliseconds / 1000) / 3600;
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

void CutVideoPage::OnCutClicked() {
    QString inputPath = inputFileLineEdit->text();
    QString outputPath = outputFileLineEdit->text();

    if (inputPath.isEmpty() || outputPath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please select input and output files."));
        return;
    }

    if (endTime <= startTime) {
        QMessageBox::warning(this, tr("Error"), tr("End time must be after start time."));
        return;
    }

    // Pause video playback during cutting
    if (videoPlayer->IsPlaying()) {
        videoPlayer->Pause();
    }

    // Create encode parameters
    EncodeParameter *encodeParam = new EncodeParameter();
    ProcessParameter *processParam = new ProcessParameter();

    // Register this page as observer for progress updates
    processParam->add_observer(this);

    // Set start and end time (convert milliseconds to seconds)
    encodeParam->SetStartTime(startTime / 1000.0);
    encodeParam->SetEndTime(endTime / 1000.0);

    // Use copy mode for fast cutting (no re-encoding)
    // Leave video and audio codec empty to copy streams

    // Show progress bar
    progressBar->setValue(0);
    progressBar->setVisible(true);
    progressLabel->setText("Starting video cutting...");
    progressLabel->setVisible(true);

    // Disable button
    cutButton->setEnabled(false);
    cutButton->setText(tr("Cutting..."));

    // Run cutting in a separate thread
    RunCutInThread(inputPath, outputPath, encodeParam, processParam);
}

void CutVideoPage::RunCutInThread(const QString &inputPath, const QString &outputPath,
                                  EncodeParameter *encodeParam, ProcessParameter *processParam) {
    QThread *thread = QThread::create([this, inputPath, outputPath, encodeParam, processParam]() {
        // Create converter
        Converter *converter = new Converter(processParam, encodeParam);
        converter->set_transcoder("FFMPEG");

        // Perform cutting
        bool success = converter->convert_format(inputPath.toStdString(), outputPath.toStdString());

        // Clean up converter
        delete converter;

        // Emit signal to notify completion
        emit CutComplete(success);
    });

    // Clean up thread when it finishes
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, [processParam, encodeParam]() {
        delete processParam;
        delete encodeParam;
    });

    thread->start();
}

void CutVideoPage::OnCutFinished(bool success) {
    // Hide progress bar
    progressBar->setVisible(false);
    progressLabel->setVisible(false);

    // Re-enable button
    cutButton->setEnabled(true);
    cutButton->setText(tr("Cut Video"));

    if (success) {
        QMessageBox::information(this, tr("Success"), tr("Video cut successfully!"));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to cut video."));
    }
}

void CutVideoPage::on_process_update(double progress) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, progress]() {
        progressBar->setValue(static_cast<int>(progress));
    }, Qt::QueuedConnection);
}

void CutVideoPage::on_time_update(double timeRequired) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, timeRequired]() {
        int minutes = static_cast<int>(timeRequired) / 60;
        int seconds = static_cast<int>(timeRequired) % 60;
        progressLabel->setText(QString("Estimated time remaining: %1:%2")
                               .arg(minutes)
                               .arg(seconds, 2, 10, QChar('0')));
    }, Qt::QueuedConnection);
}

void CutVideoPage::RetranslateUi() {
    // Update all translatable strings
    inputGroupBox->setTitle(tr("Input File"));
    inputFileLineEdit->setPlaceholderText(tr("Select a video file..."));
    browseInputButton->setText(tr("Browse..."));

    durationGroupBox->setTitle(tr("Media Duration"));
    totalDurationLabel->setText(tr("Total Duration:"));

    playerGroupBox->setTitle(tr("Video Player"));
    if (videoPlayer && videoPlayer->IsPlaying()) {
        playPauseButton->setText(tr("Pause"));
    } else {
        playPauseButton->setText(tr("Play"));
    }

    timeSelectionGroupBox->setTitle(tr("Time Selection"));
    startTimeLabel->setText(tr("Start Time:"));
    setStartButton->setText(tr("Set from Player"));
    endTimeLabel->setText(tr("End Time:"));
    setEndButton->setText(tr("Set from Player"));
    cutDurationLabel->setText(tr("Cut Duration:"));

    // Update dynamic duration values
    UpdateDurationLabel();

    outputGroupBox->setTitle(tr("Output File"));
    outputFileLineEdit->setPlaceholderText(tr("Output file path..."));
    browseOutputButton->setText(tr("Browse..."));
    cutButton->setText(tr("Cut Video"));
}
