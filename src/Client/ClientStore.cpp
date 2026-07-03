#include "ClientStore.h"

namespace edu {

ClientInstance* g_ClientInstance = nullptr;

namespace {
std::atomic<bool> g_captured{ false };
}

void captureClientInstance(ClientInstance* instance) {
    if (!instance) return;
    bool expected = false;
    if (g_captured.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
        g_ClientInstance = instance;
}

ClientInstance* getClientInstance() {
    return g_captured.load(std::memory_order_acquire) ? g_ClientInstance : nullptr;
}

bool hasClientInstance() {
    return g_captured.load(std::memory_order_acquire);
}

} // namespace edu
