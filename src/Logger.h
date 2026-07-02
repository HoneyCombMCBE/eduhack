#pragma once
// Minimal thread-safe logger that allocates its own console and mirrors every
// line to OutputDebugString so it is also visible in a debugger, if attached.

#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <mutex>

class Logger {
public:
    static Logger& instance() {
        static Logger s;
        return s;
    }

    // Allocate a brand-new console window. Safe to call from any thread; the
    // first caller wins. No-op if a console is already attached.
    void openConsole() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_consoleReady) return;

        if (!AllocConsole()) {
            // May fail if the host process already has a console; try attaching.
            AttachConsole(ATTACH_PARENT_PROCESS);
        }

        // Give the window a useful title.
        SetConsoleTitleA("eduHacksss :: ClientInstance logger");

        // Re-open the standard streams so printf-style IO hits the new console.
        freopen_s(&m_pConOut, "CONOUT$", "w", stdout);
        freopen_s(&m_pConIn,  "CONIN$",  "r", stdin);
        freopen_s(&m_pConErr, "CONERR$", "w", stderr);

        // Brighter visibility: clear screen + green text.
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (h && h != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        }

        m_consoleReady = true;
    }

    // printf-style line writer. Thread-safe, newline-terminated, also echoed
    // to OutputDebugString for live debugging.
    void log(const char* fmt, ...) {
        std::lock_guard<std::mutex> lock(m_mutex);

        char timeBuf[24] = { 0 };
        SYSTEMTIME st;
        GetLocalTime(&st);
        std::snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u:%02u.%03u ",
                      st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

        char body[1024];
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(body, sizeof(body), fmt, args);
        va_end(args);

        char line[1280];
        std::snprintf(line, sizeof(line), "[%s] %s\r\n", timeBuf, body);

        if (m_consoleReady) {
            fputs(line, stdout);
            fflush(stdout);
        }
        OutputDebugStringA(line);
    }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex  m_mutex;
    FILE*       m_pConOut   = nullptr;
    FILE*       m_pConIn    = nullptr;
    FILE*       m_pConErr   = nullptr;
    bool        m_consoleReady = false;
};

// Convenience macro so call sites read naturally: LOG("scan hit @ %p", ptr);
#define LOG(...) Logger::instance().log(__VA_ARGS__)
