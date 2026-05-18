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

#include "logger.h"

#include <chrono>
#include <cstdarg>
#include <ctime>

extern "C" {
#include <libavutil/log.h>
}

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    m_originalCallback = av_log_default_callback;
}

Logger::~Logger() {
    SetEnabled(false);
}

void Logger::SetLogPath(const std::string& path) {
    m_logPath = path;
}

std::string Logger::GetLogPath() const {
    return m_logPath;
}

bool Logger::IsEnabled() const {
    return m_enabled;
}

void Logger::SetEnabled(bool enabled) {
    if (enabled == m_enabled)
        return;

    if (enabled) {
        m_logFile.open(m_logPath, std::ios::app);
        if (!m_logFile.is_open())
            return; // path not set or unwritable — stay disabled
        m_enabled = true;
        av_log_set_callback(&Logger::AvLogCallback);
    } else {
        m_enabled = false;
        av_log_set_callback(m_originalCallback);
        if (m_logFile.is_open())
            m_logFile.close();
    }
}

const char* Logger::LevelToString(int level) {
    if (level <= AV_LOG_FATAL)   return "FATAL";
    if (level <= AV_LOG_ERROR)   return "ERROR";
    if (level <= AV_LOG_WARNING) return "WARNING";
    if (level <= AV_LOG_INFO)    return "INFO";
    return "DEBUG";
}

void Logger::AvLogCallback(void* avcl, int level, const char* fmt, va_list vl) {
    Logger& logger = Instance();

    // Copy va_list for our use; forward the original to the saved callback
    va_list vl_copy;
    va_copy(vl_copy, vl);

    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, vl_copy);
    va_end(vl_copy);

    // Timestamp
    auto now       = std::chrono::system_clock::now();
    std::time_t t  = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_info = std::localtime(&t);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_info);

    if (logger.m_logFile.is_open()) {
        logger.m_logFile << "[" << timeBuf << "] ["
                         << LevelToString(level) << "] " << buf;
        logger.m_logFile.flush();
    }

    // Forward to original callback so console output still works
    if (logger.m_originalCallback)
        logger.m_originalCallback(avcl, level, fmt, vl);
}
