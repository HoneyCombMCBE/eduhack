#pragma once
// Thread-safe console/file logger. Owns an AllocConsole window and mirrors
// every line to OutputDebugString for live debugging.

#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <mutex>
#include <string>

namespace edu {

class Log {
public:
    static Log& instance() {
        static Log s;
        return s;
    }

    // Allocate a new console window for the host process. Idempotent.
    void openConsole() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_consoleReady) return;

        if (!AllocConsole()) {
            // Already has a console (e.g. injected into a console host); attach.
            AttachConsole(ATTACH_PARENT_PROCESS);
        }

        SetConsoleTitleA("eduHacksss :: log");

        FILE* dummy = nullptr;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONIN$",  "r", stdin);
        freopen_s(&dummy, "CONERR$", "w", stderr);

        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (h && h != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        }

        banner();
        m_consoleReady = true;
    }

    void log(const char* level, const char* fmt, ...) {
        std::lock_guard<std::mutex> lock(m_mutex);

        char timeBuf[16] = { 0 };
        SYSTEMTIME st; GetLocalTime(&st);
        std::snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u:%02u.%03u",
                      st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

        char body[1024];
        va_list args; va_start(args, fmt);
        std::vsnprintf(body, sizeof(body), fmt, args);
        va_end(args);

        char line[1300];
        std::snprintf(line, sizeof(line), "[%s] [%-5s] %s\r\n", timeBuf, level, body);

        if (m_consoleReady) {
            fputs(line, stdout);
            fflush(stdout);
        }
        OutputDebugStringA(line);
    }

private:
    void banner() {
        const char* b =
            "\r\n"
            "==============================================\r\n"
            "  eduHacksss - ClientInstance logger scaffold\r\n"
            "==============================================\r\n";
        fputs(b, stdout);
        fflush(stdout);
        OutputDebugStringA(b);
    }

    Log()  = default;
    ~Log() = default;
    Log(const Log&)            = delete;
    Log& operator=(const Log&) = delete;

    std::mutex m_mutex;
    bool       m_consoleReady = false;
};

} // namespace edu

#define LOG_OPEN()  ::edu::Log::instance().openConsole()
#define LOG_INFO(...)  ::edu::Log::instance().log("INFO",  __VA_ARGS__)
#define LOG_WARN(...)  ::edu::Log::instance().log("WARN",  __VA_ARGS__)
#define LOG_ERROR(...) ::edu::Log::instance().log("ERROR", __VA_ARGS__)
// Debug-level (very chatty per-frame) lines; easy to #ifdef off later.
#define LOG_DEBUG(...) ::edu::Log::instance().log("DEBUG", __VA_ARGS__)
