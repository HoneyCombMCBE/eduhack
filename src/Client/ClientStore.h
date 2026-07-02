#pragma once
// ClientStore: utility layer over the live ClientInstance. Owns the global
// pointer and the capture/get helpers. Kept separate from the ClientInstance
// type itself (which is a pure type definition, not a service).

#include "ClientInstance.h"
#include <atomic>

namespace edu {

// The captured live ClientInstance, or nullptr before update() first runs.
// Raw pointer for direct reinterpretation of game memory.
extern ClientInstance* g_ClientInstance;

// Capture the live instance. The first non-null call wins; subsequent calls
// are ignored (ClientInstance is a long-lived singleton in the game).
void captureClientInstance(ClientInstance* instance);

// Returns the captured instance, or nullptr if not yet captured.
ClientInstance* getClientInstance();

// True once an instance has been captured.
bool hasClientInstance();

} // namespace edu
