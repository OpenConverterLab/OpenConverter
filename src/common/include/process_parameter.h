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

#ifndef PROCESSPARAMETER_H
#define PROCESSPARAMETER_H

#include "process_observer.h"
#include <memory>
#include <vector>

class ProcessParameter {
public:
    ProcessParameter();
    ~ProcessParameter();

    void set_process_number(int64_t frameNumber, int64_t frameTotalNumnber);
    void set_process_number(int64_t processNumber);
    double get_process_number();
    void set_time_required(double timeRequired);
    double get_time_required();
    ProcessParameter get_process_parmeter();

    // Observer management
    void add_observer(std::shared_ptr<ProcessObserver> observer);
    void remove_observer(std::shared_ptr<ProcessObserver> observer);

private:
    int64_t processNumber;
    double timeRequired;
    std::vector<std::shared_ptr<ProcessObserver>> observers;

    void notify_process_update(double progress);
    void notify_time_update(double timeRequired);
};

#endif // PROCESSPARAMETER_H
