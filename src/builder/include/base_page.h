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

#ifndef BASE_PAGE_H
#define BASE_PAGE_H

#include <QWidget>

class BasePage : public QWidget {
    Q_OBJECT

public:
    explicit BasePage(QWidget *parent = nullptr);
    virtual ~BasePage();

    // Virtual method to be called when page becomes active
    virtual void OnPageActivated();

    // Virtual method to be called when page becomes inactive
    virtual void OnPageDeactivated();

    // Get page title
    virtual QString GetPageTitle() const = 0;
};

#endif // BASE_PAGE_H
