#include "../Memory/Hooks.h"
#include "../Client/IClient.h"
#include "../Client/ClientStore.h"
#include "../Client/ModuleManager.h"
#include "../Features/Hooks/ClientInstanceUpdate.h"
#include "../Features/Hooks/LevelTick.h"
#include "../Features/Fly.h"
#include "../Features/CoordsDisplay.h"
#include "../Features/Sprint.h"
#include "../Features/ArrayList.h"
#include "../Features/KillAura.h"
#include "../Features/ClickTp.h"
#include "../Rendering/SwapChainHook.h"

#include <windows.h>

using namespace edu;

static HMODULE g_hModule = nullptr;
volatile bool g_disable = false;

static DWORD WINAPI init(LPVOID) {
    g_disable = false;

    edu::getModules().clear();

    features::g_flyEnabled = false;
    features::g_sprintEnabled = false;
    features::g_killAuraEnabled = false;
    features::g_arrayListEnabled = false;
    features::g_coordsEnabled = false;
    features::g_clickTpEnabled = false;
    features::g_killAuraRange = 20;
    features::g_killAuraDelay = 2;
    features::g_killAuraMulti = 0;
    features::g_killAuraTargets = 0;

    if (!Hooks::init()) return 1;

    features::hooks::ClientInstanceUpdate::install();

    registerModule("Fly", "Toggle creative flight mode", "Movement",
                   &features::g_flyEnabled, []{ features::toggleFly(); });

    registerModule("Sprint", "Always sprint when moving", "Movement",
                   &features::g_sprintEnabled, []{ features::toggleSprint(); });

    registerModule("ClickTp", "Teleport to looked block", "Movement",
                   &features::g_clickTpEnabled, []{ features::toggleClickTp(); });

    {
        edu::ModuleSetting range;
        range.name = "Range";
        range.type = edu::SettingType::Slider;
        range.selected = &features::g_killAuraRange;
        range.min = 1;
        range.max = 50;
        range.step = 1;

        edu::ModuleSetting delay;
        delay.name = "Delay";
        delay.type = edu::SettingType::Slider;
        delay.selected = &features::g_killAuraDelay;
        delay.min = 0;
        delay.max = 20;
        delay.step = 1;

        edu::ModuleSetting multi;
        multi.name = "MultiAura";
        multi.type = edu::SettingType::Dropdown;
        multi.options = {"Single", "Multi"};
        multi.selected = &features::g_killAuraMulti;

        edu::ModuleSetting targets;
        targets.name = "Targets";
        targets.type = edu::SettingType::Dropdown;
        targets.options = {"All", "Players", "Mobs"};
        targets.selected = &features::g_killAuraTargets;

        registerModule("KillAura", "Attack nearby entities", "Combat",
                       &features::g_killAuraEnabled, []{ features::toggleKillAura(); },
                       {range, delay, multi, targets});
    }

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
    edu::resetClientInstance();

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
