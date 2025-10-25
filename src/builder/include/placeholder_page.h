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

#ifndef PLACEHOLDER_PAGE_H
#define PLACEHOLDER_PAGE_H

#include "base_page.h"
#include <QLabel>
#include <QVBoxLayout>

class PlaceholderPage : public BasePage {
    Q_OBJECT

public:
    explicit PlaceholderPage(const QString &title, QWidget *parent = nullptr);
    ~PlaceholderPage();

    QString GetPageTitle() const override;

private:
    QString pageTitle;
    QLabel *label;
};

#endif // PLACEHOLDER_PAGE_H
