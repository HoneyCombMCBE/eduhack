#include "ClientStore.h"
#include <windows.h>

namespace edu {

ClientInstance* g_ClientInstance = nullptr;
static HWND g_hwnd = nullptr;

namespace {
std::atomic<bool> g_captured{ false };
}

void resetClientInstance() {
    g_ClientInstance = nullptr;
    g_captured.store(false, std::memory_order_release);
}

void captureClientInstance(ClientInstance* instance) {
    if (!instance) return;
    bool expected = false;
    if (g_captured.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        g_ClientInstance = instance;
        g_hwnd = GetForegroundWindow();
    }
}

ClientInstance* getClientInstance() {
    return g_captured.load(std::memory_order_acquire) ? g_ClientInstance : nullptr;
}

bool hasClientInstance() {
    return g_captured.load(std::memory_order_acquire);
}


} // namespace edu
