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

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

extern "C" {
#include <libavutil/log.h>
}

class Logger {
public:
    static Logger& Instance();

    // Set the full path for the log file (must be called before SetEnabled(true))
    void SetLogPath(const std::string& path);
    std::string GetLogPath() const;

    // Enable or disable file logging
    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    // Non-copyable singleton
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    static void AvLogCallback(void* avcl, int level, const char* fmt, va_list vl);
    static const char* LevelToString(int level);

    bool          m_enabled  = false;
    int           m_minLevel = AV_LOG_DEBUG; // reserved: future per-level filtering
    std::string   m_logPath;
    std::ofstream m_logFile;
    void (*m_originalCallback)(void*, int, const char*, va_list) = nullptr;
};

#endif // LOGGER_H
