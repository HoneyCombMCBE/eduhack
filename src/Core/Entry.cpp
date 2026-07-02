#include "../Memory/Hooks.h"
#include "../Client/IClient.h"
#include "../Client/ModuleManager.h"
#include "../Features/Hooks/ClientInstanceUpdate.h"
#include "../Features/Fly.h"
#include "../Rendering/SwapChainHook.h"

#include <windows.h>

using namespace edu;

static DWORD WINAPI bootstrap(LPVOID) {
    if (!Hooks::init()) return 1;

    features::hooks::ClientInstanceUpdate::install();

    registerModule("Fly", "Toggle creative flight mode", &features::g_flyEnabled,
                   []{ features::toggleFly(); });

    return 0;
}

static HANDLE g_thread = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        g_thread = CreateThread(nullptr, 0, &bootstrap, nullptr, 0, nullptr);
        if (g_thread) CloseHandle(g_thread);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        rendering::removeSwapChainHook();
        features::hooks::ClientInstanceUpdate::remove();
        Hooks::shutdown();
    }
    return TRUE;
}
