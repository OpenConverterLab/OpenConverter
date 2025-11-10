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

#ifndef PROGRESS_WIDGET_H
#define PROGRESS_WIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief Reusable progress widget component
 *
 * Encapsulates the common pattern of QProgressBar + QLabel for progress tracking.
 * Used across all conversion pages (ExtractAudioPage, RemuxPage, CutVideoPage, TranscodePage).
 *
 * Features:
 * - Progress bar (0-100 range)
 * - Progress label (for time remaining or status messages)
 * - Initially hidden, shown during conversion
 * - Provides direct access to bar and label for ConverterRunner
 *
 * Usage:
 * @code
 * ProgressWidget *progressWidget = new ProgressWidget(this);
 * mainLayout->addWidget(progressWidget);
 *
 * // Access components for ConverterRunner
 * converterRunner = new ConverterRunner(
 *     progressWidget->GetProgressBar(),
 *     progressWidget->GetProgressLabel(),
 *     actionButton, ...
 * );
 * @endcode
 */
class ProgressWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit ProgressWidget(QWidget *parent = nullptr);

    /**
     * @brief Get the progress bar component
     * @return Pointer to QProgressBar
     */
    QProgressBar *GetProgressBar() const { return progressBar; }

    /**
     * @brief Get the progress label component
     * @return Pointer to QLabel
     */
    QLabel *GetProgressLabel() const { return progressLabel; }

    /**
     * @brief Show the progress widget
     */
    void Show();

    /**
     * @brief Hide the progress widget
     */
    void Hide();

    /**
     * @brief Reset progress to 0 and clear label
     */
    void Reset();

private:
    QProgressBar *progressBar;
    QLabel *progressLabel;
    QVBoxLayout *layout;
};

#endif // PROGRESS_WIDGET_H
