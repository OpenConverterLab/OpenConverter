#include "theme_utils.h"

#include <QSettings>
#include <QProcess>
#include <QString>
#include <QFile>

bool isSystemDarkTheme() {
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                       QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;

#elif defined(Q_OS_MAC)
    QProcess proc;
    proc.start("/usr/bin/defaults", QStringList() << "read" << "-g" << "AppleInterfaceStyle");
    proc.waitForFinished();
    QString output = proc.readAllStandardOutput().trimmed();
    qDebug() << "stderr : " << proc.readAllStandardError();
    qDebug() << "output : " << output;
    return output.compare("Dark", Qt::CaseInsensitive) == 0;

#elif defined(Q_OS_LINUX)
    QProcess proc;
    proc.start("/usr/bin/gsettings", QStringList() << "get" << "org.gnome.desktop.interface" << "color-scheme");
    proc.waitForFinished();
    QString output = proc.readAllStandardOutput().trimmed();
    qDebug() << "stderr : " << proc.readAllStandardError();
    qDebug() << "output : " << output;
    return output.contains("dark", Qt::CaseInsensitive);

#else
    return false;
#endif
}

void loadStyleSheet(QApplication &app, const QString &name) {
    QString qssPath = QString(":/qss/%1.qss").arg(name);
    QFile file(qssPath);
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
    } else {
        qWarning("Failed to load QSS file: %s", qPrintable(qssPath));
    }
}
