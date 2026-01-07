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

#ifndef PYTHON_MANAGER_H
#define PYTHON_MANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>

/**
 * @brief Manages embedded Python runtime for OpenConverter
 *
 * This class handles:
 * - Detecting if Python is installed in app bundle
 * - Downloading Python 3.9 standalone build
 * - Installing Python packages from requirements.txt
 * - Providing isolated Python environment (no system conflicts)
 *
 * Similar to how Blender, GIMP, and other apps bundle Python.
 */
class PythonManager : public QObject {
    Q_OBJECT

public:
    enum class Status {
        NotInstalled,      // Python not found in app bundle
        Installing,        // Currently downloading/installing
        Installed,         // Python installed and ready
        Error              // Installation failed
    };

    explicit PythonManager(QObject *parent = nullptr);
    ~PythonManager();

    /**
     * @brief Check if Python is installed in app bundle
     * @return true if Python.framework exists and is functional
     */
    bool IsPythonInstalled();

    /**
     * @brief Check if all required packages are installed
     * @return true if all packages from requirements.txt are available
     */
    bool ArePackagesInstalled();

    /**
     * @brief Check if system Python 3.9 with required packages exists
     * @return true if system Python 3.9 has all required packages (Debug mode optimization)
     */
    bool CheckSystemPython();

    /**
     * @brief Get path to embedded Python executable
     * @return Path to python3 binary, or empty string if not installed
     */
    QString GetPythonPath();

    /**
     * @brief Get path to site-packages directory
     * @return Path to site-packages, or empty string if not installed
     */
    QString GetSitePackagesPath();

    /**
     * @brief Get current installation status
     */
    Status GetStatus() const { return status; }

    /**
     * @brief Get installation progress (0-100)
     */
    int GetProgress() const { return progress; }

    /**
     * @brief Get current status message
     */
    QString GetStatusMessage() const { return statusMessage; }

public slots:
    /**
     * @brief Download and install Python 3.9 to app bundle
     *
     * Downloads Python standalone build from python.org
     * Extracts to Contents/Frameworks/Python.framework
     * Emits signals for progress updates
     */
    void InstallPython();

    /**
     * @brief Install packages from requirements.txt
     *
     * Uses bundled pip to install packages
     * Packages are installed to embedded site-packages
     * Does not affect system Python
     */
    void InstallPackages();

    /**
     * @brief Cancel ongoing installation
     */
    void CancelInstallation();

signals:
    /**
     * @brief Emitted when installation status changes
     */
    void StatusChanged(PythonManager::Status status);

    /**
     * @brief Emitted during download/installation
     * @param progress Progress percentage (0-100)
     * @param message Status message
     */
    void ProgressChanged(int progress, const QString &message);

    /**
     * @brief Emitted when Python installation completes successfully
     */
    void PythonInstalled();

    /**
     * @brief Emitted when package installation completes successfully
     */
    void PackagesInstalled();

    /**
     * @brief Emitted when installation fails
     * @param error Error message
     */
    void InstallationFailed(const QString &error);

private slots:
    void OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void OnDownloadFinished();
    void OnInstallProcessOutput();
    void OnInstallProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString GetAppBundlePath();
    QString GetPythonFrameworkPath();
    QString GetRequirementsPath();
    bool ExtractPythonArchive(const QString &archivePath);
    bool CopyDirectoryRecursively(const QString &source, const QString &destination);
    void SetStatus(Status newStatus, const QString &message);
    void SetProgress(int value, const QString &message);

    QNetworkAccessManager *networkManager;
    QNetworkReply *currentDownload;
    QProcess *installProcess;

    Status status;
    int progress;
    QString statusMessage;
    QString downloadedFilePath;
};

#endif // PYTHON_MANAGER_H
