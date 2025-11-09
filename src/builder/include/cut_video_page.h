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

#ifndef CUT_VIDEO_PAGE_H
#define CUT_VIDEO_PAGE_H

#include "base_page.h"
#include "file_selector_widget.h"
#include "simple_video_player.h"
#include "../../common/include/process_observer.h"
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QThread>
#include <QTimeEdit>

class EncodeParameter;
class ProcessParameter;

class CutVideoPage : public BasePage, public ProcessObserver {
    Q_OBJECT

public:
    explicit CutVideoPage(QWidget *parent = nullptr);
    ~CutVideoPage() override;

    void OnPageActivated() override;
    void OnPageDeactivated() override;
    QString GetPageTitle() const override { return "Cut Video"; }
    void RetranslateUi() override;

    // ProcessObserver interface
    void on_process_update(double progress) override;
    void on_time_update(double timeRequired) override;

protected:
    void OnInputFileChanged(const QString &newPath) override;
    void OnOutputPathUpdate() override;

private slots:
    void OnInputFileSelected(const QString &filePath);
    void OnOutputFileSelected(const QString &filePath);
    void OnCutClicked();
    void OnPlayPauseClicked();
    void OnVideoPlayerPositionChanged(qint64 position);
    void OnVideoPlayerDurationChanged(qint64 duration);
    void OnTimelineSliderPressed();
    void OnTimelineSliderReleased();
    void OnTimelineSliderMoved(int position);
    void OnStartTimeChanged(const QTime &time);
    void OnEndTimeChanged(const QTime &time);
    void OnSetStartClicked();
    void OnSetEndClicked();
    void OnCutFinished(bool success);

signals:
    void CutComplete(bool success);

private:
    void SetupUI();
    void UpdateOutputPath();
    void LoadVideo(const QString &filePath);
    void UpdateTimeLabels();
    void UpdateDurationLabel();
    QString FormatTime(qint64 milliseconds);
    void RunCutInThread(const QString &inputPath, const QString &outputPath,
                        EncodeParameter *encodeParam, ProcessParameter *processParam);

    // Input/Output section
    FileSelectorWidget *inputFileSelector;
    FileSelectorWidget *outputFileSelector;

    // Media Duration section
    QGroupBox *durationGroupBox;
    QLabel *totalDurationLabel;
    QLabel *totalDurationValueLabel;

    // Video Player section
    QGroupBox *playerGroupBox;
    SimpleVideoPlayer *videoPlayer;
    QPushButton *playPauseButton;
    QSlider *timelineSlider;
    QLabel *currentTimeLabel;
    QLabel *endTimeDisplayLabel;
    bool isSliderPressed;

    // Time Selection section
    QGroupBox *timeSelectionGroupBox;
    QLabel *startTimeLabel;
    QTimeEdit *startTimeEdit;
    QPushButton *setStartButton;
    QLabel *endTimeLabel;
    QTimeEdit *endTimeEdit;
    QPushButton *setEndButton;
    QLabel *cutDurationLabel;
    QLabel *cutDurationValueLabel;

    // Progress section
    QProgressBar *progressBar;
    QLabel *progressLabel;

    // Action section
    QPushButton *cutButton;

    // State
    qint64 videoDuration;  // in milliseconds
    qint64 startTime;      // in milliseconds
    qint64 endTime;        // in milliseconds
};

#endif // CUT_VIDEO_PAGE_H
