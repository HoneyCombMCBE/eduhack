#include "../Memory/Hooks.h"
#include "../Client/IClient.h"
#include "../Client/ModuleManager.h"
#include "../Features/Hooks/ClientInstanceUpdate.h"
#include "../Features/Fly.h"
#include "../Rendering/SwapChainHook.h"

#include <windows.h>

using namespace edu;

static HMODULE g_hModule = nullptr;
volatile bool g_disable = false;

static DWORD WINAPI init(LPVOID) {
    if (!Hooks::init()) return 1;

    features::hooks::ClientInstanceUpdate::install();

    registerModule("Fly", "Toggle creative flight mode", "Movement",
                   &features::g_flyEnabled, []{ features::toggleFly(); });

    while (!g_disable) {
        if (GetAsyncKeyState(VK_END) & 1) {
            g_disable = true;
            break;
        }
        Sleep(10);
    }

    rendering::removeSwapChainHook();
    features::hooks::ClientInstanceUpdate::remove();
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    FreeLibraryAndExitThread(g_hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        CloseHandle(CreateThread(nullptr, 0, init, hModule, 0, nullptr));
    }
    return TRUE;
}
