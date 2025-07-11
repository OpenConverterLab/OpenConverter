#include "../include/process_parameter.h"
#include <algorithm>

ProcessParameter::ProcessParameter() : processNumber(0), timeRequired(0.0) {}

ProcessParameter::~ProcessParameter() = default;

void ProcessParameter::set_Process_Number(int64_t frameNumber,
                                          int64_t frameTotalNumnber) {
    if (frameTotalNumnber > 0) {
        double progress =
            static_cast<double>(frameNumber) / frameTotalNumnber * 100.0;
        processNumber = frameNumber;
        notifyProcessUpdate(progress);
    }
}

void ProcessParameter::set_Process_Number(int64_t processNumber) {
    this->processNumber = processNumber;
    notifyProcessUpdate(static_cast<double>(processNumber));
}

double ProcessParameter::get_Process_Number() {
    return static_cast<double>(processNumber);
}

void ProcessParameter::set_Time_Required(double timeRequired) {
    this->timeRequired = timeRequired;
    notifyTimeUpdate(timeRequired);
}

double ProcessParameter::get_Time_Required() { return timeRequired; }

ProcessParameter ProcessParameter::get_Process_Parmeter() { return *this; }

void ProcessParameter::addObserver(std::shared_ptr<ProcessObserver> observer) {
    if (observer) {
        observers.push_back(observer);
    }
}

void ProcessParameter::removeObserver(
    std::shared_ptr<ProcessObserver> observer) {
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end()) {
        observers.erase(it);
    }
}

void ProcessParameter::notifyProcessUpdate(double progress) {
    for (const auto &observer : observers) {
        observer->onProcessUpdate(progress);
    }
}

void ProcessParameter::notifyTimeUpdate(double timeRequired) {
    for (const auto &observer : observers) {
        observer->onTimeUpdate(timeRequired);
    }
}
