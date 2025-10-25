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

#include "../include/placeholder_page.h"

PlaceholderPage::PlaceholderPage(const QString &title, QWidget *parent)
    : BasePage(parent), pageTitle(title) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    label = new QLabel(QString("Page: %1\n\nThis page will be implemented later.").arg(title), this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px; color: #666;");

    layout->addWidget(label);
    setLayout(layout);
}

PlaceholderPage::~PlaceholderPage() {}

QString PlaceholderPage::GetPageTitle() const {
    return pageTitle;
}
