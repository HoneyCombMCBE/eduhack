#pragma once
// Thin wrapper around MinHook. Centralizes init/shutdown and exposes a small,
// type-safe CreateHook helper so feature code never touches MinHook directly.

#include <windows.h>
#include "../../third_party/minhook/include/MinHook.h"

namespace edu {

class Hooks {
    inline static bool s_init = false;

public:
    // Initialize MinHook. Safe to call once at startup. Returns false if
    // MinHook itself failed to initialize (out of memory etc.).
    static bool init() {
        if (s_init) return true;
        MH_STATUS s = MH_Initialize();
        if (s != MH_OK) {
            return false;
        }
        s_init = true;
        return true;
    }

    // Tear down all hooks (disables + removes them), then shut MinHook down.
    static void shutdown() {
        if (!s_init) return;
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        s_init = false;
    }

    // Enable a single hook (by target address).
    static bool enable(void* target) {
        return MH_EnableHook(target) == MH_OK;
    }

    // Enable every queued hook.
    static bool enableAll() {
        return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
    }

    // Create a hook: target -> detour, fills 'original' with a callable trampoline.
    // Returns true on success.
    template <typename Fn>
    static bool create(void* target, void* detour, Fn** original) {
        return MH_CreateHook(target, detour,
                             reinterpret_cast<LPVOID*>(original)) == MH_OK;
    }
};

} // namespace edu
