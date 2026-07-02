#include "Log.h"
#include "../Memory/Hooks.h"
#include "../Client/IClient.h"
#include "../Features/Hooks/ClientInstanceUpdate.h"

#include <windows.h>

using namespace edu;

// Bootstrap runs on a worker thread so we never touch the loader lock in
// DllMain. We open the console, init hooks, install features, then idle -
// holding the thread alive for the lifetime of the inject.
static DWORD WINAPI bootstrap(LPVOID) {
    LOG_OPEN();
    LOG_INFO("edu client injected. PID=%lu", GetCurrentProcessId());

    if (!Hooks::init()) {
        LOG_ERROR("MinHook failed to initialize; aborting bootstrap.");
        return 1;
    }
    LOG_INFO("MinHook initialized.");

    if (!features::hooks::ClientInstanceUpdate::install()) {
        LOG_WARN("ClientInstanceUpdate hook did not install. "
                 "The signature may not be loaded yet; the client will run "
                 "without it.");
    }

    LOG_INFO("Bootstrap complete. Waiting for ClientInstance::update()...");
    return 0;
}

static HANDLE g_thread = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        // Keep the DLL mapped for the life of the process.
        g_thread = CreateThread(nullptr, 0, &bootstrap, nullptr, 0, nullptr);
        if (g_thread) CloseHandle(g_thread);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        // Best-effort teardown. Hooks may already be gone if the host is exiting.
        features::hooks::ClientInstanceUpdate::remove();
        Hooks::shutdown();
    }
    return TRUE;
}
