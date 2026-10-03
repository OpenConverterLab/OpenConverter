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

#include "updater.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

static const char *kGitHubApiUrl =
    "https://api.github.com/repos/OpenConverterLab/OpenConverter/releases/latest";

Updater::Updater(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)), m_silent(true) {
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &Updater::OnReleaseInfoReceived);
}

Updater::~Updater() = default;

void Updater::CheckForUpdates(bool silent) {
    m_silent = silent;
    QNetworkRequest request(QUrl(kGitHubApiUrl));
    request.setRawHeader("Accept", "application/vnd.github.v3+json");
    request.setRawHeader("User-Agent", "OpenConverter");
    m_manager->get(request);
}

void Updater::OnReleaseInfoReceived(QNetworkReply *reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        if (!m_silent) {
            emit CheckFailed(reply->errorString());
        }
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) {
        if (!m_silent) {
            emit CheckFailed(tr("Invalid response from GitHub"));
        }
        return;
    }

    QJsonObject release = doc.object();
    QString tagName = release.value("tag_name").toString();
    QString releaseNotes = release.value("body").toString();
    QString downloadUrl = ExtractPlatformDownloadUrl(release);

    if (tagName.isEmpty()) {
        if (!m_silent) {
            emit CheckFailed(tr("No release found"));
        }
        return;
    }

    QString currentVer = OC_VERSION;
    QString latestVer = tagName.startsWith('v') ? tagName.mid(1) : tagName;

    if (IsNewerVersion(latestVer, currentVer)) {
        emit UpdateAvailable(currentVer, latestVer, downloadUrl, releaseNotes);
    } else {
        emit NoUpdateAvailable();
    }
}

bool Updater::IsNewerVersion(const QString &remote, const QString &local) {
    QVersionNumber remoteVer = QVersionNumber::fromString(remote);
    QVersionNumber localVer = QVersionNumber::fromString(local);
    return remoteVer > localVer;
}

QString Updater::ExtractPlatformDownloadUrl(const QJsonObject &release) {
    QJsonArray assets = release.value("assets").toArray();

    // 根据平台匹配文件名关键字
    QString keyword;
#ifdef Q_OS_WIN
    keyword = "win";
#elif defined(Q_OS_MACOS)
    keyword = "macos";
#elif defined(Q_OS_LINUX)
    keyword = "linux";
#endif

    // 优先匹配平台关键字，其次匹配通用安装包
    for (const QJsonValue &val : assets) {
        QJsonObject asset = val.toObject();
        QString name = asset.value("name").toString().toLower();
        if (!keyword.isEmpty() && name.contains(keyword)) {
            return asset.value("browser_download_url").toString();
        }
    }

    // 回退：找 dmg / exe / msi / AppImage
    for (const QJsonValue &val : assets) {
        QJsonObject asset = val.toObject();
        QString name = asset.value("name").toString().toLower();
        if (name.endsWith(".dmg") || name.endsWith(".exe") ||
            name.endsWith(".msi") || name.endsWith(".appimage")) {
            return asset.value("browser_download_url").toString();
        }
    }

    // 最后回退到 release 页面
    return release.value("html_url").toString();
}
