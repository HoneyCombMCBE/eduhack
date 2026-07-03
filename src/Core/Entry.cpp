#include "../Memory/Hooks.h"
#include "../Client/IClient.h"
#include "../Client/ModuleManager.h"
#include "../Features/Hooks/ClientInstanceUpdate.h"
#include "../Features/Fly.h"
#include "../Rendering/SwapChainHook.h"

#include <windows.h>
#include <cstdio>

using namespace edu;

static HMODULE g_hModule = nullptr;
volatile bool g_disable = false;

static bool g_consoleReady = false;
static std::string g_lastScreen;

static void initConsole() {
    if (g_consoleReady) return;
    AllocConsole();
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleTitleA("heheboi :: debug");
    g_consoleReady = true;
    printf("[heheboi] ready\n");
    fflush(stdout);
}

static void pollScreen() {
    auto* ci = getClientInstance();
    if (!ci) return;
    std::string screen = ci->getScreenName();
    if (screen != g_lastScreen) {
        printf("[screen] %s -> %s\n", g_lastScreen.c_str(), screen.c_str());
        fflush(stdout);
        g_lastScreen = screen;
    }
}

static DWORD WINAPI init(LPVOID) {
    if (!Hooks::init()) return 1;

    features::hooks::ClientInstanceUpdate::install();

    registerModule("Fly", "Toggle creative flight mode", "Movement",
                   &features::g_flyEnabled, []{ features::toggleFly(); });

    initConsole();

    while (!g_disable) {
        if (GetAsyncKeyState(VK_END) & 1) {
            g_disable = true;
            break;
        }
        pollScreen();
        Sleep(50);
    }

    if (g_consoleReady) FreeConsole();

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
