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

#ifndef CONVERTER_RUNNER_H
#define CONVERTER_RUNNER_H

#include "../../common/include/process_observer.h"
#include <QLabel>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <functional>

class EncodeParameter;
class ProcessParameter;

/**
 * @brief Helper class to manage conversion operations with progress tracking
 *
 * This class encapsulates the common pattern of:
 * - Input validation
 * - Progress bar management
 * - Button state management
 * - Thread-based conversion execution
 * - Completion handling with success/error messages
 *
 * Usage:
 * @code
 * ConverterRunner *runner = new ConverterRunner(
 *     progressBar, progressLabel, convertButton,
 *     "Converting...", "Convert",
 *     "Success", "File converted successfully!",
 *     "Error", "Failed to convert file.",
 *     this
 * );
 *
 * connect(runner, &ConverterRunner::ConversionFinished, this, &MyPage::OnConversionFinished);
 *
 * runner->RunConversion(inputPath, outputPath, encodeParam, processParam);
 * @endcode
 */
class ConverterRunner : public QObject, public ProcessObserver {
    Q_OBJECT

public:
    /**
     * @brief Construct a ConverterRunner
     * @param progressBar Progress bar widget to show conversion progress
     * @param progressLabel Label to show time remaining
     * @param actionButton Button to trigger conversion (will be disabled during conversion)
     * @param runningButtonText Text to show on button during conversion (e.g., "Converting...")
     * @param idleButtonText Text to show on button when idle (e.g., "Convert")
     * @param successTitle Title for success message box
     * @param successMessage Message for success message box
     * @param errorTitle Title for error message box
     * @param errorMessage Message for error message box
     * @param parent Parent QObject
     */
    explicit ConverterRunner(QProgressBar *progressBar,
                            QLabel *progressLabel,
                            QPushButton *actionButton,
                            const QString &runningButtonText,
                            const QString &idleButtonText,
                            const QString &successTitle,
                            const QString &successMessage,
                            const QString &errorTitle,
                            const QString &errorMessage,
                            QObject *parent = nullptr);

    ~ConverterRunner() override;

    /**
     * @brief Run conversion in a separate thread
     * @param inputPath Input file path
     * @param outputPath Output file path
     * @param encodeParam Encoding parameters (ownership transferred to runner)
     * @param processParam Process parameters (ownership transferred to runner)
     * @param transcoderName Transcoder name (default: "FFMPEG")
     * @return true if conversion started successfully, false if validation failed
     */
    bool RunConversion(const QString &inputPath,
                      const QString &outputPath,
                      EncodeParameter *encodeParam,
                      ProcessParameter *processParam,
                      const QString &transcoderName = "FFMPEG");

    /**
     * @brief Set custom validation function
     * @param validator Function that returns true if validation passes, false otherwise
     */
    void SetValidator(std::function<bool()> validator);

    /**
     * @brief Set custom completion handler (called before showing message box)
     * @param handler Function called with success status
     */
    void SetCompletionHandler(std::function<void(bool)> handler);

    // ProcessObserver interface
    void on_process_update(double progress) override;
    void on_time_update(double timeRequired) override;

signals:
    /**
     * @brief Emitted when conversion finishes
     * @param success true if conversion succeeded, false otherwise
     */
    void ConversionFinished(bool success);

private:
    void ShowProgressUI();
    void HideProgressUI();
    void SetButtonRunning();
    void SetButtonIdle();
    void ShowCompletionMessage(bool success);

    QProgressBar *progressBar;
    QLabel *progressLabel;
    QPushButton *actionButton;
    QString runningButtonText;
    QString idleButtonText;
    QString successTitle;
    QString successMessage;
    QString errorTitle;
    QString errorMessage;

    std::function<bool()> customValidator;
    std::function<void(bool)> customCompletionHandler;
};

#endif // CONVERTER_RUNNER_H
