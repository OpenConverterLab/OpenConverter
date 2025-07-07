#ifndef THEME_UTILS_H
#define THEME_UTILS_H

#include <QApplication>

bool isSystemDarkTheme();
void loadStyleSheet(QApplication& app, const QString& name);

#endif // THEME_UTILS_H
