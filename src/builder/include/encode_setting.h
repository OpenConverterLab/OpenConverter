/*
 * Copyright 2024 Jack Lau
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

#ifndef ENCODE_SETTING_H
#define ENCODE_SETTING_H

#include "../../common/include/encode_parameter.h"
#include <QMainWindow>

namespace Ui {
class EncodeSetting;
}

class EncodeSetting : public QMainWindow {
    Q_OBJECT
protected:
    // this event is called, when a new translator is loaded or the system
    // language is changed
    void changeEvent(QEvent *);

public:
    explicit EncodeSetting(QWidget *parent = nullptr,
                           EncodeParameter *encodeParamter = NULL);

    ~EncodeSetting();

    bool get_Encode_Parameter(EncodeParameter *ep);

    bool get_Available();

public slots:
    void cancel_Pushed();

    void apply_Pushed();

private:
    EncodeParameter *encodeParameter = NULL;

    Ui::EncodeSetting *ui;
};

#endif // ENCODE_SETTING_H
