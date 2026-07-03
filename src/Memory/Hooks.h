#pragma once
#include <windows.h>
#include "../../third_party/minhook/include/MinHook.h"

namespace edu {

class Hooks {
    inline static bool s_init = false;
public:
    static bool init() {
        if (s_init) return true;
        if (MH_Initialize() != MH_OK) return false;
        s_init = true;
        return true;
    }

    static void shutdown() {
        if (!s_init) return;
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        s_init = false;
    }

    static bool enable(void* target) { return MH_EnableHook(target) == MH_OK; }
    static bool enableAll() { return MH_EnableHook(MH_ALL_HOOKS) == MH_OK; }

    template <typename Fn>
    static bool create(void* target, void* detour, Fn** original) {
        return MH_CreateHook(target, detour, reinterpret_cast<LPVOID*>(original)) == MH_OK;
    }
};

} // namespace edu
