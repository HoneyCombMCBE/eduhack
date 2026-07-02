#include "ClientStore.h"

namespace edu {

// Defined here, not in a header, to keep the symbol in a single TU.
// Initial value: no instance captured yet.
ClientInstance* g_ClientInstance = nullptr;

namespace {
// One-time capture flag. We keep the global pointer + a flag so that the
// pointer write is observable even if the game recreates the object (it
// doesn't for ClientInstance, but being explicit avoids surprises).
std::atomic<bool> g_captured{ false };
} // namespace

void captureClientInstance(ClientInstance* instance) {
    if (!instance) return;
    // First writer wins.
    bool expected = false;
    if (g_captured.compare_exchange_strong(expected, true,
                                           std::memory_order_acq_rel)) {
        g_ClientInstance = instance;
    }
}

ClientInstance* getClientInstance() {
    return g_captured.load(std::memory_order_acquire) ? g_ClientInstance : nullptr;
}

bool hasClientInstance() {
    return g_captured.load(std::memory_order_acquire);
}

} // namespace edu
