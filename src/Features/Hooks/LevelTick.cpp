#include "LevelTick.h"

#include "../../Memory/Hooks.h"
#include "../../Client/ClientInstance.h"
#include "../../Client/ClientStore.h"
#include "../Fly.h"
#include "../Sprint.h"

#include <MinHook.h>

namespace edu::features::hooks::LevelTick {

static constexpr size_t kTickWorldIdx = 180;

using TickWorldFn = void(__fastcall*)(void*, const void*);
static TickWorldFn o_tickWorld = nullptr;
static void* g_target = nullptr;

static void __fastcall hk_tickWorld(void* player, const void* tick) {
    o_tickWorld(player, tick);

    edu::features::tickFly(player);
    edu::features::tickSprint(player);
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
