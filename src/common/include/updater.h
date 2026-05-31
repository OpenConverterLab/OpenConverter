/*
 * Copyright 2026 Finch
 * Email: 1418875140@qq.com
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

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVersionNumber>

class Updater : public QObject {
    Q_OBJECT

public:
    explicit Updater(QObject *parent = nullptr);
    ~Updater();

    // silent=true: 启动时静默检查，有更新才弹窗
    // silent=false: 手动检查，无更新也提示
    void CheckForUpdates(bool silent);

signals:
    void UpdateAvailable(const QString &currentVer, const QString &latestVer,
                         const QString &downloadUrl, const QString &releaseNotes);
    void NoUpdateAvailable();
    void CheckFailed(const QString &errorMsg);

private slots:
    void OnReleaseInfoReceived(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    bool m_silent;

    static bool IsNewerVersion(const QString &remote, const QString &local);
    static QString ExtractPlatformDownloadUrl(const QJsonObject &release);
};

#endif // UPDATER_H
