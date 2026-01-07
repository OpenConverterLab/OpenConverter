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

#ifndef PYTHON_INSTALL_DIALOG_H
#define PYTHON_INSTALL_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include "python_manager.h"

/**
 * @brief Dialog for installing Python runtime and packages
 *
 * Shows when user tries to use AI Processing but Python is not installed.
 * Provides user-friendly interface for downloading and installing Python.
 */
class PythonInstallDialog : public QDialog {
    Q_OBJECT

public:
    explicit PythonInstallDialog(QWidget *parent = nullptr);
    ~PythonInstallDialog();

    /**
     * @brief Check if installation was successful
     */
    bool WasSuccessful() const { return installSuccess; }

private slots:
    void OnInstallClicked();
    void OnCancelClicked();
    void OnStatusChanged(PythonManager::Status status);
    void OnProgressChanged(int progress, const QString &message);
    void OnPythonInstalled();
    void OnPackagesInstalled();
    void OnInstallationFailed(const QString &error);

private:
    void SetupUI();
    void RetranslateUi();

    PythonManager *pythonManager;

    QLabel *titleLabel;
    QLabel *descriptionLabel;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    QPushButton *installButton;
    QPushButton *cancelButton;

    bool installSuccess;
    bool installingPython;  // true = installing Python, false = installing packages
};

#endif // PYTHON_INSTALL_DIALOG_H
