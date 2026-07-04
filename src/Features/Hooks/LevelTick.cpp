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

    char buf[256];
    std::snprintf(buf, sizeof(buf), "vtable[187]: %p  vtable[188]: %p",
        ptr187, ptr188);
    edu::logChat(buf);

    if (ptr187) {
        float* tryView = reinterpret_cast<float*>((char*)ptr187 + 0x380);
        std::snprintf(buf, sizeof(buf), "187+0x380: %.2f %.2f %.2f %.2f",
            tryView[0], tryView[1], tryView[2], tryView[3]);
        edu::logChat(buf);
    }

    if (ptr188) {
        float* tryView = reinterpret_cast<float*>((char*)ptr188 + 0x380);
        std::snprintf(buf, sizeof(buf), "188+0x380: %.2f %.2f %.2f %.2f",
            tryView[0], tryView[1], tryView[2], tryView[3]);
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
