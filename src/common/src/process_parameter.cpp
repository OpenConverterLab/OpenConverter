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

#include "../include/process_parameter.h"
#include <algorithm>

ProcessParameter::ProcessParameter() : processNumber(0), timeRequired(0.0) {}

ProcessParameter::~ProcessParameter() = default;

void ProcessParameter::set_process_number(int64_t frameNumber,
                                          int64_t frameTotalNumnber) {
    if (frameTotalNumnber > 0) {
        double progress =
            static_cast<double>(frameNumber) / frameTotalNumnber * 100.0;
        processNumber = frameNumber;
        notify_process_update(progress);
    }
}

void ProcessParameter::set_process_number(int64_t processNumber) {
    this->processNumber = processNumber;
    notify_process_update(static_cast<double>(processNumber));
}

double ProcessParameter::get_process_number() {
    return static_cast<double>(processNumber);
}

void ProcessParameter::set_time_required(double timeRequired) {
    this->timeRequired = timeRequired;
    notify_time_update(timeRequired);
}

double ProcessParameter::get_time_required() { return timeRequired; }

ProcessParameter ProcessParameter::get_process_parmeter() { return *this; }

void ProcessParameter::add_observer(ProcessObserver* observer) {
    if (observer) {
        observers.push_back(observer);
    }
}

void ProcessParameter::remove_observer(ProcessObserver* observer) {
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end()) {
        observers.erase(it);
    }
}

void ProcessParameter::notify_process_update(double progress) {
    for (auto observer : observers) {
        if (observer) {
            observer->on_process_update(progress);
        }
    }
}

void ProcessParameter::notify_time_update(double timeRequired) {
    for (auto observer : observers) {
        if (observer) {
            observer->on_time_update(timeRequired);
        }
    }
}
