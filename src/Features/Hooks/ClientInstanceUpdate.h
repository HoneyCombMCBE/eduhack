#pragma once
// Feature: hook ClientInstance::update(bool isInitFinished).
//
// On the first call, the live `this` is captured into g_ClientInstance. Every
// call is logged. This is the entry point for the whole client: once we hold
// the instance, every other virtual (getLocalPlayer, getLevel, ...) is
// reachable through its vtable.

namespace edu::features::hooks::ClientInstanceUpdate {

// Scan for the update() function, install the detour, enable the hook.
// Returns true on success.
bool install();

// Disable and remove the hook.
void remove();

} // namespace edu::features::hooks::ClientInstanceUpdate
