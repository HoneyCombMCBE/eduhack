#pragma once
// IClient: the public API surface of the edu client. The single header the
// rest of the (future) client depends on. Today it exposes lifecycle and the
// captured ClientInstance; it is extended feature by feature.

#include "ClientInstance.h"
#include "ClientStore.h"

namespace edu {

class IClient {
public:
    // One-time bootstrap: init hooks, run the sig scan, install hooks.
    // Returns true if initialization completed (the instance itself is
    // captured later, when update() first runs).
    static bool initialize();

    // Graceful teardown: remove hooks, free resources.
    static void shutdown();

    // Access the captured game ClientInstance. nullptr until update() runs.
    static ClientInstance* clientInstance() { return getClientInstance(); }

    // True once the ClientInstance has been captured.
    static bool isReady() { return hasClientInstance(); }
};

} // namespace edu
