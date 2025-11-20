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

#include "python_manager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QCoreApplication>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// Python 3.9 standalone build URL (macOS)
// Using Python Standalone Builds from Astral (formerly Gregory Szorc)
// These are pre-built, relocatable Python frameworks - much easier to extract
#ifdef __APPLE__
#ifdef __aarch64__
// macOS ARM64 (Apple Silicon)
const QString PYTHON_DOWNLOAD_URL = "https://github.com/astral-sh/python-build-standalone/releases/download/20251031/cpython-3.9.25+20251031-aarch64-apple-darwin-install_only.tar.gz";
const QString PYTHON_ARCHIVE_SIZE_MB = "18";
#else
// macOS x86_64 (Intel)
const QString PYTHON_DOWNLOAD_URL = "https://github.com/astral-sh/python-build-standalone/releases/download/20251031/cpython-3.9.25+20251031-x86_64-apple-darwin-install_only.tar.gz";
const QString PYTHON_ARCHIVE_SIZE_MB = "18";
#endif
#endif

PythonManager::PythonManager(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , currentDownload(nullptr)
    , installProcess(nullptr)
    , status(Status::NotInstalled)
    , progress(0)
{
    // Check initial status
    // First check embedded Python in app bundle
    if (IsPythonInstalled()) {
        if (ArePackagesInstalled()) {
            SetStatus(Status::Installed, "Python and packages are ready");
        } else {
            SetStatus(Status::NotInstalled, "Python installed but packages missing");
        }
    } else {
#ifndef NDEBUG
        // Debug mode only: check if system Python 3.9 with required packages exists
        // This allows developers to use their existing Python environment
        if (CheckSystemPython()) {
            SetStatus(Status::Installed, "Using system Python 3.9 with required packages");
        } else {
            SetStatus(Status::NotInstalled, "Python not installed");
        }
#else
        // Release mode: Only use bundled Python, never fall back to system Python
        SetStatus(Status::NotInstalled, "Python not installed");
#endif
    }
}

PythonManager::~PythonManager() {
    if (currentDownload) {
        currentDownload->abort();
        currentDownload->deleteLater();
    }
    if (installProcess) {
        installProcess->kill();
        installProcess->deleteLater();
    }
}

QString PythonManager::GetAppBundlePath() {
#ifdef __APPLE__
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        QString exePath = QString::fromUtf8(path);
        // Extract app bundle path (everything before .app/Contents/MacOS)
        int appIndex = exePath.indexOf(".app/Contents/MacOS");
        if (appIndex != -1) {
            return exePath.left(appIndex + 4); // Include .app
        }
    }
#endif
    return QCoreApplication::applicationDirPath();
}

QString PythonManager::GetPythonFrameworkPath() {
    return GetAppBundlePath() + "/Contents/Frameworks/Python.framework";
}

QString PythonManager::GetPythonPath() {
    // Python Standalone Builds use flat structure: python/bin/python3.9
    QString pythonPath = GetPythonFrameworkPath() + "/bin/python3.9";
    if (QFile::exists(pythonPath)) {
        return pythonPath;
    }
    return QString();
}

QString PythonManager::GetSitePackagesPath() {
    // Python Standalone Builds use flat structure: python/lib/python3.9/site-packages
    QString sitePath = GetPythonFrameworkPath() + "/lib/python3.9/site-packages";
    if (QDir(sitePath).exists()) {
        return sitePath;
    }
    return QString();
}

QString PythonManager::GetRequirementsPath() {
    return GetAppBundlePath() + "/Contents/Resources/requirements.txt";
}

bool PythonManager::IsPythonInstalled() {
    QString pythonPath = GetPythonPath();
    if (pythonPath.isEmpty()) {
        return false;
    }

    // Verify Python is executable and correct version
    QProcess process;
    process.start(pythonPath, QStringList() << "--version");
    if (!process.waitForFinished(3000)) {
        return false;
    }

    QString output = process.readAllStandardOutput();
    return output.contains("Python 3.9");
}

bool PythonManager::ArePackagesInstalled() {
    // Fast check: Just verify package directories exist in site-packages
    // This is much faster than importing packages (which can take 10+ seconds)

    QString appBundlePath = GetAppBundlePath();
    if (appBundlePath.isEmpty()) {
        return false;
    }

    // Get site-packages directory
    QString sitePackages = appBundlePath + "/Contents/Frameworks/Python.framework/lib/python3.9/site-packages";

    if (!QDir(sitePackages).exists()) {
        qDebug() << "site-packages directory not found:" << sitePackages;
        return false;
    }

    // Check if key package directories exist
    QStringList requiredPackages = {"torch", "basicsr", "realesrgan", "bmf"};

    for (const QString &package : requiredPackages) {
        QString packagePath = sitePackages + "/" + package;
        if (!QDir(packagePath).exists()) {
            qDebug() << "Package directory not found:" << package << "at" << packagePath;
            return false;
        }
    }

    qDebug() << "All required packages found in site-packages";
    return true;
}

bool PythonManager::CheckSystemPython() {
    // Check if system Python 3.9 exists with all required packages
    // This is useful in Debug mode to avoid re-downloading Python
    // Note: BMF is checked separately since it's bundled with the app

    QStringList pythonCandidates = {"python3.9", "python3"};

    for (const QString &pythonCmd : pythonCandidates) {
        QProcess versionCheck;
        versionCheck.start(pythonCmd, QStringList() << "--version");
        if (!versionCheck.waitForFinished(3000)) {
            continue;
        }

        QString output = versionCheck.readAllStandardOutput();
        if (!output.contains("Python 3.9")) {
            continue;
        }

        // Found Python 3.9, now check if core AI packages are installed
        // BMF is excluded because it's bundled with the app and will be added to PYTHONPATH
        QStringList requiredPackages = {"torch", "basicsr", "realesrgan"};
        bool allPackagesFound = true;

        for (const QString &package : requiredPackages) {
            QProcess packageCheck;
            packageCheck.start(pythonCmd, QStringList() << "-c" << QString("import %1").arg(package));
            if (!packageCheck.waitForFinished(5000) || packageCheck.exitCode() != 0) {
                qDebug() << "System Python missing package:" << package;
                allPackagesFound = false;
                break;
            }
        }

        if (allPackagesFound) {
            qDebug() << "Found system Python 3.9 with required AI packages:" << pythonCmd;
            qDebug() << "BMF will be loaded from bundled location";
            return true;
        }
    }

    return false;
}

void PythonManager::SetStatus(Status newStatus, const QString &message) {
    status = newStatus;
    statusMessage = message;
    emit StatusChanged(status);
    qDebug() << "PythonManager status:" << message;
}

void PythonManager::SetProgress(int value, const QString &message) {
    progress = value;
    statusMessage = message;
    emit ProgressChanged(progress, statusMessage);
}

void PythonManager::InstallPython() {
    if (status == Status::Installing) {
        qWarning() << "Installation already in progress";
        return;
    }

    SetStatus(Status::Installing, "Downloading Python 3.9...");
    SetProgress(0, "Starting download...");

    // Download Python installer
    QUrl url(PYTHON_DOWNLOAD_URL);
    QNetworkRequest request(url);

    // Follow redirects (GitHub releases redirect to CDN)
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                        QNetworkRequest::NoLessSafeRedirectPolicy);

    currentDownload = networkManager->get(request);

    connect(currentDownload, &QNetworkReply::downloadProgress,
            this, &PythonManager::OnDownloadProgress);
    connect(currentDownload, &QNetworkReply::finished,
            this, &PythonManager::OnDownloadFinished);
}

void PythonManager::OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        int percent = (bytesReceived * 50) / bytesTotal; // 0-50% for download
        SetProgress(percent, QString("Downloading Python: %1 MB / %2 MB")
                    .arg(bytesReceived / 1024 / 1024)
                    .arg(bytesTotal / 1024 / 1024));
    }
}

void PythonManager::OnDownloadFinished() {
    if (!currentDownload) {
        return;
    }

    if (currentDownload->error() != QNetworkReply::NoError) {
        QString error = currentDownload->errorString();
        currentDownload->deleteLater();
        currentDownload = nullptr;
        SetStatus(Status::Error, "Download failed: " + error);
        emit InstallationFailed(error);
        return;
    }

    // Save downloaded file
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    downloadedFilePath = tempDir + "/python-3.9.25.tar.gz";

    QFile file(downloadedFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QString error = "Failed to save archive: " + file.errorString();
        currentDownload->deleteLater();
        currentDownload = nullptr;
        SetStatus(Status::Error, error);
        emit InstallationFailed(error);
        return;
    }

    QByteArray data = currentDownload->readAll();
    if (data.isEmpty()) {
        QString error = "Downloaded file is empty (0 bytes). Check network connection.";
        file.close();
        currentDownload->deleteLater();
        currentDownload = nullptr;
        SetStatus(Status::Error, error);
        emit InstallationFailed(error);
        return;
    }

    file.write(data);
    file.close();

    qDebug() << "Downloaded Python archive:" << downloadedFilePath
             << "Size:" << (data.size() / 1024 / 1024) << "MB";

    currentDownload->deleteLater();
    currentDownload = nullptr;

    SetProgress(50, "Download complete. Extracting Python...");

    // Extract Python from archive
    if (!ExtractPythonArchive(downloadedFilePath)) {
        QString error = "Failed to extract Python";
        SetStatus(Status::Error, error);
        emit InstallationFailed(error);
        return;
    }

    SetProgress(100, "Python installation complete");
    SetStatus(Status::Installed, "Python installed successfully");
    emit PythonInstalled();

    // Clean up downloaded file
    QFile::remove(downloadedFilePath);
}

bool PythonManager::ExtractPythonArchive(const QString &archivePath) {
    // Extract .tar.gz archive using tar command
    // Python Standalone Builds have structure: python/install/...
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString extractDir = tempDir + "/python_extract";

    // Clean up any previous extraction
    QDir(extractDir).removeRecursively();
    QDir().mkpath(extractDir);

    // Extract tar.gz
    QProcess process;
    process.start("tar", QStringList()
                  << "-xzf"
                  << archivePath
                  << "-C"
                  << extractDir);

    if (!process.waitForFinished(120000)) {  // 2 minutes timeout
        qWarning() << "tar extraction timeout";
        return false;
    }

    if (process.exitCode() != 0) {
        qWarning() << "tar extraction failed:" << process.readAllStandardError();
        return false;
    }

    // Python Standalone Builds extract to: python/
    QString extractedPython = extractDir + "/python";
    if (!QDir(extractedPython).exists()) {
        qWarning() << "Extracted Python not found at:" << extractedPython;
        return false;
    }

    // Move to final location
    QString targetPath = GetPythonFrameworkPath();
    QString targetParent = QFileInfo(targetPath).path();
    QDir().mkpath(targetParent);

    // Remove existing Python.framework if it exists
    QDir(targetPath).removeRecursively();

    // Move extracted Python to Python.framework
    if (!QFile::rename(extractedPython, targetPath)) {
        qWarning() << "Failed to move Python to:" << targetPath;
        return false;
    }

    // Clean up extraction directory
    QDir(extractDir).removeRecursively();

    qDebug() << "Python extracted successfully to:" << targetPath;
    return true;
}

void PythonManager::InstallPackages() {
    QString pythonPath = GetPythonPath();
    if (pythonPath.isEmpty()) {
        QString error = "Python not installed";
        SetStatus(Status::Error, error);
        emit InstallationFailed(error);
        return;
    }

    QString requirementsPath = GetRequirementsPath();
    if (!QFile::exists(requirementsPath)) {
        QString error = "requirements.txt not found";
        SetStatus(Status::Error, error);
        emit InstallationFailed(error);
        return;
    }

    SetStatus(Status::Installing, "Installing Python packages...");
    SetProgress(0, "Installing packages from requirements.txt...");

    // Install packages using pip
    installProcess = new QProcess(this);
    connect(installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonManager::OnInstallProcessFinished);
    connect(installProcess, &QProcess::readyReadStandardOutput,
            this, &PythonManager::OnInstallProcessOutput);
    connect(installProcess, &QProcess::readyReadStandardError,
            this, &PythonManager::OnInstallProcessOutput);

    // Set working directory to /tmp to avoid macOS permission issues
    installProcess->setWorkingDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation));

    // Merge stdout and stderr for better progress tracking
    installProcess->setProcessChannelMode(QProcess::MergedChannels);

    installProcess->start(pythonPath, QStringList()
                         << "-m" << "pip" << "install"
                         << "-r" << requirementsPath
                         << "--no-cache-dir"
                         << "--progress-bar" << "on");
}

void PythonManager::OnInstallProcessOutput() {
    if (!installProcess) return;

    QString output = installProcess->readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        // Parse pip progress: "Downloading package-name (X.X MB)"
        // or "Installing collected packages: ..."
        if (line.contains("Downloading") || line.contains("Installing")) {
            // Simple progress estimation based on output
            static int packageCount = 0;
            packageCount++;
            int progress = qMin(90, packageCount * 10); // Cap at 90% until finished
            SetProgress(progress, line.trimmed());
        }
    }
}

void PythonManager::OnInstallProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        QString error = "Package installation failed: " + installProcess->readAll();
        SetStatus(Status::Error, error);
        emit InstallationFailed(error);
        installProcess->deleteLater();
        installProcess = nullptr;
        return;
    }

    // Pip install succeeded, now copy BMF Python bindings
    SetProgress(95, "Copying BMF Python bindings...");

    if (!CopyBMFPythonBindings()) {
        QString error = "Failed to copy BMF Python bindings";
        SetStatus(Status::Error, error);
        emit InstallationFailed(error);
        installProcess->deleteLater();
        installProcess = nullptr;
        return;
    }

    SetProgress(100, "All packages installed successfully");
    SetStatus(Status::Installed, "Python and packages ready");
    emit PackagesInstalled();

    installProcess->deleteLater();
    installProcess = nullptr;
}

void PythonManager::CancelInstallation() {
    if (currentDownload) {
        currentDownload->abort();
    }
    if (installProcess) {
        installProcess->kill();
    }
    SetStatus(Status::NotInstalled, "Installation cancelled");
}

bool PythonManager::CopyBMFPythonBindings() {
    // BMF Python package is bundled in the app's Resources/bmf_python/
    // We need to copy it to the embedded Python's site-packages

    QString embeddedSitePackages = GetSitePackagesPath();
    if (embeddedSitePackages.isEmpty()) {
        qWarning() << "Embedded site-packages not found";
        return false;
    }

    // Get bundled BMF Python package path
    QString appBundlePath = GetAppBundlePath();
    QString bmfSourcePath = appBundlePath + "/Contents/Resources/bmf_python";

    if (!QDir(bmfSourcePath).exists()) {
        qWarning() << "Bundled BMF Python package not found at:" << bmfSourcePath;
        return false;
    }

    // Copy BMF directory to embedded site-packages
    QString bmfDestPath = embeddedSitePackages + "/bmf";

    // Remove existing BMF if present
    QDir(bmfDestPath).removeRecursively();

    // Copy BMF directory recursively
    if (!CopyDirectoryRecursively(bmfSourcePath, bmfDestPath)) {
        qWarning() << "Failed to copy BMF from" << bmfSourcePath << "to" << bmfDestPath;
        return false;
    }

    qDebug() << "BMF Python bindings copied successfully from" << bmfSourcePath;
    return true;
}

bool PythonManager::CopyDirectoryRecursively(const QString &source, const QString &destination) {
    QDir sourceDir(source);
    if (!sourceDir.exists()) {
        return false;
    }

    QDir destDir(destination);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    // Copy all files
    QFileInfoList entries = sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &entry : entries) {
        QString srcPath = entry.absoluteFilePath();
        QString dstPath = destination + "/" + entry.fileName();

        if (entry.isDir()) {
            // Recursively copy subdirectory
            if (!CopyDirectoryRecursively(srcPath, dstPath)) {
                return false;
            }
        } else {
            // Copy file
            if (!QFile::copy(srcPath, dstPath)) {
                qWarning() << "Failed to copy file:" << srcPath << "to" << dstPath;
                return false;
            }
        }
    }

    return true;
}
