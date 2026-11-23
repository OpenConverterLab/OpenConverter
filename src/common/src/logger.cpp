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
#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

#if defined(_WIN32)
    #include <windows.h>
#endif

extern "C" {
#include <libavutil/log.h>
}

namespace {

// Qt passes UTF-8. On Windows ofstream(const char*) uses the ANSI code page,
// so convert to UTF-16 and let filesystem::path use the wide open. POSIX
// paths are already UTF-8 bytes.
std::filesystem::path PathFromUtf8(const std::string& utf8) {
#if defined(_WIN32)
    if (utf8.empty())
        return {};
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                          utf8.data(),
                                          static_cast<int>(utf8.size()),
                                          nullptr, 0);
    if (size <= 0)
        return {};
    std::wstring wide(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
                        static_cast<int>(utf8.size()), wide.data(), size);
    return std::filesystem::path(wide);
#else
    return std::filesystem::path(utf8);
#endif
}

} // namespace

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    // av_log_get_default_callback() is not in FFmpeg 5.1.
    m_originalCallback = &av_log_default_callback;
    m_savedLogLevel = av_log_get_level();
}

Logger::~Logger() = default;

void Logger::SetLogPath(const std::string& path) {
    bool reopen = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (path == m_logPath)
            return;
        m_logPath = path;
        reopen = m_enabled;
    }
    if (reopen)
        SetEnabled(true);
}

std::string Logger::GetLogPath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logPath;
}

bool Logger::IsEnabled() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_enabled && m_logFile.is_open();
}

bool Logger::FileLoggingEnabled() { return Instance().IsEnabled(); }

void Logger::CloseFileUnlocked() {
    m_enabled = false;
    m_line.clear();
    m_printPrefix = 1;
    if (m_logFile.is_open()) {
        m_logFile.flush();
        m_logFile.close();
    }
    m_logFile.clear();
    m_openPath.clear();
    if (m_callbackInstalled) {
        av_log_set_level(m_savedLogLevel);
        av_log_set_callback(m_originalCallback ? m_originalCallback
                                               : &av_log_default_callback);
        m_callbackInstalled = false;
    }
}

void Logger::ShutdownUnlocked() {
    // Keep a trailing fragment that never received its newline. Do this
    // before CloseFileUnlocked, and do not call WriteLineLocked: a failed
    // write would re-enter shutdown.
    if (m_enabled && m_logFile.is_open() && !m_line.empty()) {
        m_logFile << '[' << Timestamp() << "] [" << LevelToString(m_lineLevel)
                  << "] " << m_line << '\n';
        m_logFile.flush();
    }
    m_line.clear();
    CloseFileUnlocked();
}

bool Logger::OpenFileUnlocked(const std::string& utf8Path, std::string& error) {
    error.clear();
    if (utf8Path.empty()) {
        error = "log path is empty";
        return false;
    }

    std::error_code ec;
    const std::filesystem::path path = PathFromUtf8(utf8Path);
    const std::filesystem::path parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            error = "cannot create log directory for \"" + utf8Path +
                    "\": " + ec.message();
            return false;
        }
    }

    if (m_logFile.is_open())
        m_logFile.close();
    m_logFile.clear();
    m_logFile.open(path, std::ios::app);
    if (!m_logFile) {
        error = "cannot open log file \"" + utf8Path + "\"";
        m_openPath.clear();
        return false;
    }
    m_openPath = utf8Path;
    return true;
}

bool Logger::SetEnabled(bool enabled) {
    std::string error;
    bool ok = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!enabled) {
            ShutdownUnlocked();
            return false;
        }

        if (m_enabled && m_logFile.is_open() && m_openPath == m_logPath)
            return true;

        if (!OpenFileUnlocked(m_logPath, error)) {
            // OpenFileUnlocked may already have closed the previous file.
            CloseFileUnlocked();
            ok = false;
        } else {
            m_enabled = true;
            if (!m_callbackInstalled) {
                // Capture DEBUG for the file, but remember the previous
                // level so disabling logging puts the console back.
                m_savedLogLevel = av_log_get_level();
                av_log_set_callback(&Logger::AvLogCallback);
                av_log_set_level(AV_LOG_DEBUG);
                m_callbackInstalled = true;
            }
            ok = true;
        }
    }

    if (!error.empty())
        std::cerr << "[Logger] " << error << '\n';
    return ok;
}

std::string Logger::Timestamp() {
    const std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
#if defined(_WIN32)
    if (localtime_s(&tm, &now) != 0)
        return "0000-00-00 00:00:00";
#else
    // localtime() shares one static buffer across threads.
    if (localtime_r(&now, &tm) == nullptr)
        return "0000-00-00 00:00:00";
#endif
    char buf[32];
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm) == 0)
        return "0000-00-00 00:00:00";
    return buf;
}

const char* Logger::LevelToString(int level) {
    if (level <= AV_LOG_FATAL)
        return "FATAL";
    if (level <= AV_LOG_ERROR)
        return "ERROR";
    if (level <= AV_LOG_WARNING)
        return "WARNING";
    if (level <= AV_LOG_INFO)
        return "INFO";
    if (level <= AV_LOG_VERBOSE)
        return "VERBOSE";
    return "DEBUG";
}

void Logger::WriteLineLocked(int level, const std::string& text) {
    if (!m_enabled || !m_logFile.is_open() || text.empty())
        return;

    m_logFile << '[' << Timestamp() << "] [" << LevelToString(level) << "] "
              << text << '\n';
    m_logFile.flush();
    if (!m_logFile) {
        std::cerr << "[Logger] write failed; file logging disabled\n";
        CloseFileUnlocked();
    }
}

void Logger::AppendLocked(int level, const char* text) {
    if (!text)
        return;

    for (const char* p = text; *p != '\0'; ++p) {
        const char c = *p;
        if (c == '\r') {
            // FFmpeg progress updates rewrite the same console line.
            m_line.clear();
            continue;
        }
        if (c == '\n') {
            if (!m_line.empty())
                WriteLineLocked(m_lineLevel, m_line);
            m_line.clear();
            if (!m_enabled)
                return;
            continue;
        }
        if (m_line.empty())
            m_lineLevel = level;
        m_line.push_back(c);
    }
}

void Logger::Log(const std::string& msg, bool isError) {
    std::vector<std::string> lines;
    std::string pending;
    for (char c : msg) {
        if (c == '\r')
            continue;
        if (c == '\n') {
            if (!pending.empty())
                lines.push_back(pending);
            pending.clear();
        } else {
            pending.push_back(c);
        }
    }
    if (!pending.empty())
        lines.push_back(std::move(pending));
    if (lines.empty())
        return;

    Logger& logger = Instance();
    std::lock_guard<std::mutex> lock(logger.m_mutex);

    std::ostream& out = isError ? std::cerr : std::cout;
    for (const std::string& line : lines)
        out << line << '\n';
    out.flush();

    if (!logger.m_enabled || !logger.m_logFile.is_open())
        return;

    const int level = isError ? AV_LOG_ERROR : AV_LOG_INFO;
    for (const std::string& line : lines)
        logger.WriteLineLocked(level, line);
}

void Logger::AvLogCallback(void* avcl, int level, const char* fmt, va_list vl) {
    Logger& logger = Instance();
    if (!fmt)
        fmt = "";

    {
        std::lock_guard<std::mutex> lock(logger.m_mutex);
        if (logger.m_enabled && logger.m_logFile.is_open()) {
            // Format twice only when the line does not fit. The first call
            // updates print_prefix, so a retry must restore it or the next
            // fragment loses or repeats FFmpeg's [component] prefix.
            const int prefixAtStart = logger.m_printPrefix;
            char stack[2048];
            va_list copy;
            va_copy(copy, vl);
            const int needed = av_log_format_line2(
                avcl, level, fmt, copy, stack, static_cast<int>(sizeof(stack)),
                &logger.m_printPrefix);
            va_end(copy);

            const char* text = nullptr;
            std::string heap;
            if (needed < 0) {
                logger.m_printPrefix = prefixAtStart;
            } else if (needed < static_cast<int>(sizeof(stack))) {
                text = stack;
            } else {
                logger.m_printPrefix = prefixAtStart;
                heap.resize(static_cast<size_t>(needed) + 1);
                va_list copy2;
                va_copy(copy2, vl);
                const int wrote = av_log_format_line2(
                    avcl, level, fmt, copy2, heap.data(), needed + 1,
                    &logger.m_printPrefix);
                va_end(copy2);
                if (wrote < 0)
                    logger.m_printPrefix = prefixAtStart;
                else
                    text = heap.c_str();
            }

            if (text) {
                // A new FFmpeg line must not be glued onto a progress
                // fragment that never ended with '\n'.
                if (prefixAtStart == 1)
                    logger.m_line.clear();
                logger.AppendLocked(level, text);
            }
        }
    }

    // The global level is DEBUG while the file is capturing detail. Keep
    // that detail out of the console.
    if (level <= AV_LOG_INFO && logger.m_originalCallback)
        logger.m_originalCallback(avcl, level, fmt, vl);
}
