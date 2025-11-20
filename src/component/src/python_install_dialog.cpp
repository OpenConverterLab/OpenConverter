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

#include "python_install_dialog.h"
#include <QMessageBox>

PythonInstallDialog::PythonInstallDialog(QWidget *parent)
    : QDialog(parent)
    , pythonManager(new PythonManager(this))
    , installSuccess(false)
    , installingPython(true)
{
    SetupUI();
    RetranslateUi();

    // Connect signals
    connect(pythonManager, &PythonManager::StatusChanged,
            this, &PythonInstallDialog::OnStatusChanged);
    connect(pythonManager, &PythonManager::ProgressChanged,
            this, &PythonInstallDialog::OnProgressChanged);
    connect(pythonManager, &PythonManager::PythonInstalled,
            this, &PythonInstallDialog::OnPythonInstalled);
    connect(pythonManager, &PythonManager::PackagesInstalled,
            this, &PythonInstallDialog::OnPackagesInstalled);
    connect(pythonManager, &PythonManager::InstallationFailed,
            this, &PythonInstallDialog::OnInstallationFailed);

    connect(installButton, &QPushButton::clicked,
            this, &PythonInstallDialog::OnInstallClicked);
    connect(cancelButton, &QPushButton::clicked,
            this, &PythonInstallDialog::OnCancelClicked);
}

PythonInstallDialog::~PythonInstallDialog() {
}

void PythonInstallDialog::SetupUI() {
    setWindowTitle("Install Python Runtime");
    setMinimumWidth(500);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    titleLabel = new QLabel(this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Description
    descriptionLabel = new QLabel(this);
    descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel);

    // Status label
    statusLabel = new QLabel(this);
    statusLabel->setWordWrap(true);
    mainLayout->addWidget(statusLabel);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    installButton = new QPushButton(this);
    installButton->setDefault(true);
    buttonLayout->addWidget(installButton);

    cancelButton = new QPushButton(this);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void PythonInstallDialog::RetranslateUi() {
    titleLabel->setText(tr("AI Processing Setup"));

    descriptionLabel->setText(tr(
        "AI Processing requires Python 3.9 and additional packages (PyTorch, BasicSR, Real-ESRGAN).\n\n"
        "This will download and install:\n"
        "• Python 3.9 runtime (~18 MB)\n"
        "• AI processing packages (~500 MB)\n\n"
        "The installation is completely isolated and will not affect your system Python."
    ));

    statusLabel->setText(tr("Ready to install"));
    installButton->setText(tr("Install"));
    cancelButton->setText(tr("Cancel"));
}

void PythonInstallDialog::OnInstallClicked() {
    installButton->setEnabled(false);
    progressBar->setVisible(true);

    if (installingPython) {
        pythonManager->InstallPython();
    } else {
        pythonManager->InstallPackages();
    }
}

void PythonInstallDialog::OnCancelClicked() {
    if (pythonManager->GetStatus() == PythonManager::Status::Installing) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Cancel Installation"),
            tr("Are you sure you want to cancel the installation?"),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            pythonManager->CancelInstallation();
            reject();
        }
    } else {
        reject();
    }
}

void PythonInstallDialog::OnStatusChanged(PythonManager::Status status) {
    switch (status) {
        case PythonManager::Status::NotInstalled:
            statusLabel->setText(tr("Not installed"));
            installButton->setEnabled(true);
            break;

        case PythonManager::Status::Installing:
            statusLabel->setText(tr("Installing..."));
            installButton->setEnabled(false);
            cancelButton->setText(tr("Cancel"));
            break;

        case PythonManager::Status::Installed:
            statusLabel->setText(tr("Installation complete!"));
            installButton->setEnabled(false);
            cancelButton->setText(tr("Close"));
            break;

        case PythonManager::Status::Error:
            statusLabel->setText(tr("Installation failed"));
            installButton->setEnabled(true);
            installButton->setText(tr("Retry"));
            break;
    }
}

void PythonInstallDialog::OnProgressChanged(int progress, const QString &message) {
    progressBar->setValue(progress);
    statusLabel->setText(message);
}

void PythonInstallDialog::OnPythonInstalled() {
    // Python installed, now install packages
    installingPython = false;
    statusLabel->setText(tr("Python installed. Installing packages..."));
    pythonManager->InstallPackages();
}

void PythonInstallDialog::OnPackagesInstalled() {
    installSuccess = true;
    progressBar->setValue(100);
    statusLabel->setText(tr("Installation complete! AI Processing is now ready."));

    QMessageBox::information(
        this,
        tr("Success"),
        tr("Python and all required packages have been installed successfully.\n\n"
           "You can now use AI Processing features.")
    );

    accept();
}

void PythonInstallDialog::OnInstallationFailed(const QString &error) {
    progressBar->setVisible(false);
    installButton->setEnabled(true);
    installButton->setText(tr("Retry"));

    QMessageBox::critical(
        this,
        tr("Installation Failed"),
        tr("Failed to install Python runtime:\n\n%1\n\n"
           "Please check your internet connection and try again.").arg(error)
    );
}
