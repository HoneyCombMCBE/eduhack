#include "LevelTick.h"

#include "../../Memory/Hooks.h"
#include "../../Client/ClientInstance.h"
#include "../../Client/ClientStore.h"
#include "../../Client/Chat.h"
#include "../Fly.h"
#include "../Sprint.h"

#include <MinHook.h>
#include <vector>
#include <string>

namespace edu::features::hooks::LevelTick {

static constexpr size_t kTickWorldIdx = 181;

using TickWorldFn = void(__fastcall*)(void*, const void*);
static TickWorldFn o_tickWorld = nullptr;
static void* g_target = nullptr;

static void testEntityCount() {
    static int cooldown = 0;
    if (cooldown > 0) { cooldown--; return; }

    static bool prevF6 = false;
    bool f6 = (GetAsyncKeyState(VK_F6) & 0x8000) != 0;
    if (!f6 || prevF6) { prevF6 = f6; return; }
    prevF6 = f6;

    auto* ci = edu::getClientInstance();
    if (!ci) return;

    auto ciVtable = *reinterpret_cast<uintptr_t**>(ci);
    using GetLevelRendererFn = void*(__fastcall*)(const void*);
    auto getLevelRenderer = reinterpret_cast<GetLevelRendererFn>(ciVtable[187]);
    void* lr = getLevelRenderer(ci);

    if (!lr) { edu::logChat("getLevelRenderer: null"); return; }
    edu::logChat("getLevelRenderer: OK");

    void* gr = *reinterpret_cast<void**>((char*)lr + 0x3F8);
    if (!gr) { edu::logChat("GameRenderer: null"); return; }
    edu::logChat("GameRenderer: OK");

    float* viewMatrix = reinterpret_cast<float*>((char*)gr + 0x380);
    float* projMatrix = reinterpret_cast<float*>((char*)gr + 0x400);

    char buf[128];
    std::snprintf(buf, sizeof(buf), "view[0]: %.2f %.2f %.2f %.2f",
        viewMatrix[0], viewMatrix[1], viewMatrix[2], viewMatrix[3]);
    edu::logChat(buf);
    std::snprintf(buf, sizeof(buf), "proj[0]: %.2f %.2f %.2f %.2f",
        projMatrix[0], projMatrix[1], projMatrix[2], projMatrix[3]);
    edu::logChat(buf);

    cooldown = 20;
}

static void __fastcall hk_tickWorld(void* player, const void* tick) {
    o_tickWorld(player, tick);

    edu::features::tickFly(player);
    edu::features::tickSprint(player);
    testEntityCount();
}

bool install() {
    auto* ci = edu::getClientInstance();
    if (!ci) return false;
    void* player = ci->getLocalPlayer();
    if (!player) return false;

    auto vtable = *reinterpret_cast<uintptr_t**>(player);
    g_target = reinterpret_cast<void*>(vtable[kTickWorldIdx]);

    if (!Hooks::create(g_target, reinterpret_cast<void*>(&hk_tickWorld), &o_tickWorld))
        return false;
    if (!Hooks::enable(g_target))
        return false;

    return true;
}

void remove() {
    if (g_target) {
        MH_DisableHook(g_target);
        MH_RemoveHook(g_target);
    }
}

} // namespace edu::features::hooks::LevelTick
