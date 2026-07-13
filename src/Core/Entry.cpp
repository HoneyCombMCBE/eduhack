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
#include "../Features/Watermark.h"
#include "../Features/PacketLogger.h"
#include "../Features/NoFall.h"
#include "../Features/Criticals.h"
#include "../Features/Derp.h"

#include "../Features/NoSlow.h"
#include "../Features/AntiHunger.h"
#include "../Features/Blink.h"
#include "../Features/Crasher.h"
#include "../Features/Velocity.h"
#include "../Features/Freecam.h"
#include "../Features/InstaBuild.h"
#include "../Client/ConfigManager.h"
#include "../Client/Theme.h"
#include "../GUI/ClickGUI.h"
#include "../GUI/Notifications.h"
#include "../Rendering/SwapChainHook.h"

#include <windows.h>

using namespace edu;

int edu::g_activeTheme = 1;

static HMODULE g_hModule = nullptr;
volatile bool g_disable = false;

static DWORD WINAPI init(LPVOID) {
    g_disable = false;

    edu::getModules().clear();
    edu::getCategoriesList().clear();

    features::g_flyEnabled = false;
    features::g_flyMode = 0;
    features::g_flySpeed = 1;
    features::g_sprintEnabled = false;
    features::g_killAuraEnabled = false;
    features::g_arrayListEnabled = false;
    features::g_coordsEnabled = false;
    features::g_watermarkEnabled = true;
    features::g_packetLoggerEnabled = false;
    features::g_noFallEnabled = false;
    features::g_noFallMode = 0;
    features::g_criticalsEnabled = false;
    features::g_criticalsMode = 0;
    features::g_criticalsRange = 5;
    features::g_derpEnabled = false;
    features::g_noSlowEnabled = false;
    features::g_killAuraRange = 20;
    features::g_killAuraDelay = 2;
    features::g_killAuraMulti = 0;
    features::g_killAuraTargets = 0;
    edu::features::g_antiHungerEnabled = false;
    features::g_blinkEnabled = false;
    features::g_blinkMode = 0;
    features::g_blinkDelay = 10;
    features::g_crasherEnabled = false;
    features::g_velocityEnabled = false;
    features::g_velocityMode = 2;
    features::g_velocityHorizontal = 0;
    features::g_velocityVertical = 0;
    features::g_freecamEnabled = false;
    features::g_instaBuildEnabled = false;

    if (!Hooks::init()) return 1;

    features::hooks::ClientInstanceUpdate::install();

    {
        edu::ModuleSetting flyMode;
        flyMode.name = "Mode";
        flyMode.type = edu::SettingType::Dropdown;
        flyMode.options = {"Creative", "Motion", "Elytra"};
        flyMode.selected = &features::g_flyMode;

        edu::ModuleSetting flySpeed;
        flySpeed.name = "Speed";
        flySpeed.type = edu::SettingType::Slider;
        flySpeed.min = 1;
        flySpeed.max = 50;
        flySpeed.selected = &features::g_flySpeed;

        registerModule("Fly", "Toggle flight mode", "Movement",
                       &features::g_flyEnabled, []{ features::toggleFly(); features::installPacketSendHook(); },
                       {flyMode, flySpeed});
    }

    registerModule("Sprint", "Always sprint when moving", "Movement",
                   &features::g_sprintEnabled, []{ features::toggleSprint(); });

    {
        edu::ModuleSetting range;
        range.name = "Range";
        range.type = edu::SettingType::Slider;
        range.selected = &features::g_killAuraRange;
        range.min = 1;
        range.max = 15;
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

    {
        edu::ModuleSetting theme;
        theme.name = "Theme";
        theme.type = edu::SettingType::Dropdown;
        theme.options = {"Rainbow", "Sunset", "Orange"};
        theme.selected = &edu::g_activeTheme;

        registerModule("Watermark", "Show watermark on screen", "Render",
                       &features::g_watermarkEnabled, []{ features::toggleWatermark(); },
                       {theme});
    }

    registerModule("PacketLogger", "Log outgoing packets to chat", "Misc",
                   &features::g_packetLoggerEnabled, []{ features::togglePacketLogger(); });

    {
        edu::ModuleSetting noFallMode;
        noFallMode.name = "Mode";
        noFallMode.type = edu::SettingType::Dropdown;
        noFallMode.options = {"Sentinel", "BDS", "Horion"};
        noFallMode.selected = &features::g_noFallMode;

        registerModule("NoFall", "Prevent fall damage", "Player",
                       &features::g_noFallEnabled, []{ features::toggleNoFall(); features::installPacketSendHook(); },
                       {noFallMode});
    }

    {
        edu::ModuleSetting critMode;
        critMode.name = "Mode";
        critMode.type = edu::SettingType::Dropdown;
        critMode.options = {"Sentinel", "Safe"};
        critMode.selected = &features::g_criticalsMode;

        edu::ModuleSetting range;
        range.name = "Range";
        range.type = edu::SettingType::Slider;
        range.selected = &features::g_criticalsRange;
        range.min = 1;
        range.max = 15;
        range.step = 1;

        registerModule("Criticals", "Always deal critical hits", "Combat",
                       &features::g_criticalsEnabled, []{ features::toggleCriticals(); features::installPacketSendHook(); },
                       {critMode, range});
    }

    registerModule("Derp", "Spin head randomly server-side", "Misc",
                   &features::g_derpEnabled, []{ features::toggleDerp(); features::installPacketSendHook(); });

    registerModule("AntiHunger", "Slow hunger depletion", "Player",
                   &edu::features::g_antiHungerEnabled, []{ edu::features::toggleAntiHunger(); features::installPacketSendHook(); });

    registerModule("NoSlowdown", "No slowdown from items/webs", "Movement",
                   &features::g_noSlowEnabled, []{ features::toggleNoSlow(); });

    registerModule("InstaBuild", "Creative instant block placement", "World",
                   &features::g_instaBuildEnabled, []{ features::toggleInstaBuild(); });

    registerModule("Crasher", "Crash realms servers", "Misc",
                   &features::g_crasherEnabled, []{ features::toggleCrasher(); features::installPacketSendHook(); });

    {
        edu::ModuleSetting velMode;
        velMode.name = "Mode";
        velMode.type = edu::SettingType::Dropdown;
        velMode.options = {"Off", "Reverse", "Zero"};
        velMode.selected = &features::g_velocityMode;

        edu::ModuleSetting velH;
        velH.name = "Horizontal";
        velH.type = edu::SettingType::Slider;
        velH.min = 0;
        velH.max = 100;
        velH.selected = &features::g_velocityHorizontal;

        edu::ModuleSetting velV;
        velV.name = "Vertical";
        velV.type = edu::SettingType::Slider;
        velV.min = 0;
        velV.max = 100;
        velV.selected = &features::g_velocityVertical;

        registerModule("Velocity", "Reduce knockback", "Combat",
                       &features::g_velocityEnabled, []{ features::toggleVelocity(); },
                       {velMode, velH, velV});
    }

    {
        edu::ModuleSetting blinkMode;
        blinkMode.name = "Mode";
        blinkMode.type = edu::SettingType::Dropdown;
        blinkMode.options = {"Normal", "Pulse"};
        blinkMode.selected = &features::g_blinkMode;

        edu::ModuleSetting blinkDelay;
        blinkDelay.name = "Delay";
        blinkDelay.type = edu::SettingType::Slider;
        blinkDelay.min = 1;
        blinkDelay.max = 60;
        blinkDelay.selected = &features::g_blinkDelay;

        registerModule("Blink", "Freeze server-side movement", "Movement",
                       &features::g_blinkEnabled, []{ features::toggleBlink(); features::installPacketSendHook(); },
                       {blinkMode, blinkDelay});
    }

    registerModule("Freecam", "Free camera movement", "Misc",
                   &features::g_freecamEnabled, []{ features::toggleFreecam(); features::installPacketSendHook(); });

    edu::loadConfig();

    // Install hooks for any modules that were restored as enabled
    if (features::g_noFallEnabled || features::g_criticalsEnabled ||
        features::g_derpEnabled ||
        edu::features::g_antiHungerEnabled || features::g_blinkEnabled ||
        features::g_freecamEnabled || features::g_blinkEnabled) {
        features::installPacketSendHook();
    }

    while (!g_disable) {
        if (GetAsyncKeyState(VK_END) & 1) {
            g_disable = true;
            break;
        }
        Sleep(10);
    }

    // Signal all hooks to bail out immediately
    g_disable = true;
    Sleep(100);

    // Disable all hooks — MH suspends threads to safely unhook
    Hooks::shutdown();

    // Now safe to tear down resources
    rendering::removeSwapChainHook();
    features::hooks::LevelTick::remove();
    features::hooks::ClientInstanceUpdate::remove();
    features::removePacketSendHook();
    gui::reset();
    gui::notifications::clear();
    edu::saveConfig();

    Sleep(100);

    edu::resetClientInstance();

    FreeLibraryAndExitThread(g_hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, init, hModule, 0, nullptr);
        if (hThread) CloseHandle(hThread);
    } else if (reason == DLL_PROCESS_DETACH) {
        // Only save if we're unloading normally (not process termination)
        // g_disable being true means init thread already cleaned up
    }
    return TRUE;
}
