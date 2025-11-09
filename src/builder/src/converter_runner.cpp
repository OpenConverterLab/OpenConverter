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

#include "../include/converter_runner.h"
#include "../../common/include/encode_parameter.h"
#include "../../common/include/process_parameter.h"
#include "../../engine/include/converter.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QThread>

ConverterRunner::ConverterRunner(QProgressBar *progressBar,
                                 QLabel *progressLabel,
                                 QPushButton *actionButton,
                                 const QString &runningButtonText,
                                 const QString &idleButtonText,
                                 const QString &successTitle,
                                 const QString &successMessage,
                                 const QString &errorTitle,
                                 const QString &errorMessage,
                                 QObject *parent)
    : QObject(parent),
      progressBar(progressBar),
      progressLabel(progressLabel),
      actionButton(actionButton),
      runningButtonText(runningButtonText),
      idleButtonText(idleButtonText),
      successTitle(successTitle),
      successMessage(successMessage),
      errorTitle(errorTitle),
      errorMessage(errorMessage),
      customValidator(nullptr),
      customCompletionHandler(nullptr) {
}

ConverterRunner::~ConverterRunner() {
}

bool ConverterRunner::RunConversion(const QString &inputPath,
                                     const QString &outputPath,
                                     EncodeParameter *encodeParam,
                                     ProcessParameter *processParam,
                                     const QString &transcoderName) {
    // Validate inputs
    if (inputPath.isEmpty() || outputPath.isEmpty()) {
        QMessageBox::warning(qobject_cast<QWidget *>(parent()),
                           tr("Warning"),
                           tr("Please select input and output files."));
        delete encodeParam;
        delete processParam;
        return false;
    }

    // Run custom validator if set
    if (customValidator && !customValidator()) {
        delete encodeParam;
        delete processParam;
        return false;
    }

    // Register this runner as observer for progress updates
    processParam->add_observer(this);

    // Show progress UI
    ShowProgressUI();
    SetButtonRunning();

    // Run conversion in a separate thread
    QThread *thread = QThread::create([this, inputPath, outputPath, encodeParam, processParam, transcoderName]() {
        // Create converter
        Converter *converter = new Converter(processParam, encodeParam);
        converter->set_transcoder(transcoderName.toStdString());

        // Perform conversion
        bool success = converter->convert_format(inputPath.toStdString(), outputPath.toStdString());

        // Clean up converter
        delete converter;

        // Emit signal to notify completion (will be handled on main thread)
        QMetaObject::invokeMethod(this, [this, success]() {
            HideProgressUI();
            SetButtonIdle();

            // Call custom completion handler if set
            if (customCompletionHandler) {
                customCompletionHandler(success);
            }

            // Show completion message
            ShowCompletionMessage(success);

            // Emit signal
            emit ConversionFinished(success);
        }, Qt::QueuedConnection);

        // Clean up parameters
        delete encodeParam;
        delete processParam;
    });

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();

    return true;
}

void ConverterRunner::SetValidator(std::function<bool()> validator) {
    customValidator = validator;
}

void ConverterRunner::SetCompletionHandler(std::function<void(bool)> handler) {
    customCompletionHandler = handler;
}

void ConverterRunner::on_process_update(double progress) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, progress]() {
        if (progressBar) {
            progressBar->setValue(static_cast<int>(progress));
        }
    }, Qt::QueuedConnection);
}

void ConverterRunner::on_time_update(double timeRequired) {
    // Use QMetaObject::invokeMethod to ensure UI updates happen on the main thread
    QMetaObject::invokeMethod(this, [this, timeRequired]() {
        if (progressLabel) {
            int minutes = static_cast<int>(timeRequired) / 60;
            int seconds = static_cast<int>(timeRequired) % 60;
            progressLabel->setText(QString("Estimated time remaining: %1:%2")
                                   .arg(minutes)
                                   .arg(seconds, 2, 10, QChar('0')));
        }
    }, Qt::QueuedConnection);
}

void ConverterRunner::ShowProgressUI() {
    if (progressBar) {
        progressBar->setValue(0);
        progressBar->setVisible(true);
    }
    if (progressLabel) {
        progressLabel->setText(tr("Starting conversion..."));
        progressLabel->setVisible(true);
    }
}

void ConverterRunner::HideProgressUI() {
    if (progressBar) {
        progressBar->setVisible(false);
    }
    if (progressLabel) {
        progressLabel->setVisible(false);
    }
}

void ConverterRunner::SetButtonRunning() {
    if (actionButton) {
        actionButton->setEnabled(false);
        actionButton->setText(runningButtonText);
    }
}

void ConverterRunner::SetButtonIdle() {
    if (actionButton) {
        actionButton->setEnabled(true);
        actionButton->setText(idleButtonText);
    }
}

void ConverterRunner::ShowCompletionMessage(bool success) {
    QWidget *parentWidget = qobject_cast<QWidget *>(parent());
    if (!parentWidget) {
        return;
    }

    if (success) {
        QMessageBox::information(parentWidget, successTitle, successMessage);
    } else {
        QMessageBox::critical(parentWidget, errorTitle, errorMessage);
    }
}
