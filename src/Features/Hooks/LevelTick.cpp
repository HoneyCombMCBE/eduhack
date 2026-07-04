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

    using GetPtrFn = void*(__fastcall*)(const void*);
    void* ptr187 = reinterpret_cast<GetPtrFn>(ciVtable[187])(ci);
    void* ptr188 = reinterpret_cast<GetPtrFn>(ciVtable[188])(ci);

    void* gr = ptr187;
    if (!gr) { edu::logChat("GameRenderer: null"); return; }

    float* vm = reinterpret_cast<float*>((char*)gr + 0x380);
    float* pm = reinterpret_cast<float*>((char*)gr + 0x400);

    char buf[256];
    for (int row = 0; row < 4; row++) {
        std::snprintf(buf, sizeof(buf), "V%d: %.3f %.3f %.3f %.3f",
            row, vm[row*4], vm[row*4+1], vm[row*4+2], vm[row*4+3]);
        edu::logChat(buf);
    }
    for (int row = 0; row < 4; row++) {
        std::snprintf(buf, sizeof(buf), "P%d: %.3f %.3f %.3f %.3f",
            row, pm[row*4], pm[row*4+1], pm[row*4+2], pm[row*4+3]);
        edu::logChat(buf);
    }

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
