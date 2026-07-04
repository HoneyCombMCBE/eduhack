#include "../Memory/Hooks.h"
#include "../Client/IClient.h"
#include "../Client/ModuleManager.h"
#include "../Features/Hooks/ClientInstanceUpdate.h"
#include "../Features/Hooks/LevelTick.h"
#include "../Features/Fly.h"
#include "../Features/CoordsDisplay.h"
#include "../Features/Sprint.h"
#include "../Features/ArrayList.h"
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

    registerModule("Sprint", "Always sprint when moving", "Movement",
                   &features::g_sprintEnabled, []{ features::toggleSprint(); });

    registerModule("ArrayList", "Show enabled modules list", "Render",
                   &features::g_arrayListEnabled, []{ features::toggleArrayList(); });

    {
        edu::ModuleSetting pos;
        pos.name = "Position";
        pos.type = edu::SettingType::Dropdown;
        pos.options = {"Bottom Left", "Bottom Right", "Top Left", "Top Right"};
        pos.selected = &features::g_coordsPosition;

        registerModule("Coords", "Show coordinates on screen", "Render",
                       &features::g_coordsEnabled, []{ features::toggleCoords(); },
                       {pos});
    }

    while (!g_disable) {
        if (GetAsyncKeyState(VK_END) & 1) {
            g_disable = true;
            break;
        }
        Sleep(10);
    }

    rendering::removeSwapChainHook();
    features::hooks::LevelTick::remove();
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
