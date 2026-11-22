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
#include <mutex>
#include <string>

extern "C" {
#include <libavutil/log.h>
}

// Process-wide FFmpeg and application logger.
//
// File output is opt-in. Console output for Logger::Log() is not: every call
// is printed immediately, and is also appended to the log file when file
// logging is on. The av_log callback is installed only while file logging is
// on. It records FFmpeg output through av_log_format_line (so FFmpeg's own
// line prefixes and partial lines stay intact) and forwards messages at
// AV_LOG_INFO and above to the previous callback.
class Logger {
public:
    static Logger& Instance();

    // UTF-8 path. Parent directories are created when logging is enabled.
    // If logging is already on, the file is reopened at the new path.
    void SetLogPath(const std::string& path);
    std::string GetLogPath() const;

    // Returns the state actually achieved. Enabling fails (and stays off)
    // when the path is empty or the file cannot be created.
    bool SetEnabled(bool enabled);
    bool IsEnabled() const;

    // Thread-safe. isError selects stderr and an ERROR record; otherwise the
    // line goes to stdout and is recorded as INFO. Embedded newlines are split
    // into separate records. A no-op write to the file when logging is off.
    static void Log(const std::string& msg, bool isError = false);

    static bool IsLoggingEnabled();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    static void AvLogCallback(void* avcl, int level, const char* fmt, va_list vl);
    static const char* LevelToString(int level);
    static std::string Timestamp();

    void CloseFileUnlocked();
    void ShutdownUnlocked();
    bool OpenFileUnlocked(const std::string& path, std::string& error);
    void AppendLocked(int level, const char* text);
    void WriteLineLocked(int level, const std::string& text);

    bool m_enabled = false;
    bool m_callbackInstalled = false;
    int m_savedLogLevel = AV_LOG_INFO;
    int m_printPrefix = 1;
    int m_lineLevel = AV_LOG_INFO;
    std::string m_logPath;
    std::string m_openPath;
    std::ofstream m_logFile;
    std::string m_line;
    mutable std::mutex m_mutex;
    void (*m_originalCallback)(void*, int, const char*, va_list) = nullptr;
};

#endif // LOGGER_H
