#include "ClientInstanceUpdate.h"

#include "../../Memory/Hooks.h"
#include "../../Client/ClientInstance.h"
#include "../../Client/ClientStore.h"
#include "../../Rendering/SwapChainHook.h"
#include "LevelTick.h"

#include <libhat/scanner.hpp>

namespace edu::features::hooks::ClientInstanceUpdate {

constexpr auto kSig = hat::compile_signature<
    "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 "
    "48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 "
    "48 89 85 ? ? ? ? 44 0F B6 FA 48 8B F1 33 DB"
>();

using UpdateFn = bool (__fastcall*)(ClientInstance*, bool);

static UpdateFn o_update = nullptr;
static void* g_target = nullptr;
static bool g_tickHooked = false;

static bool __fastcall hk_update(ClientInstance* self, bool isInitFinished) {
    if (!edu::hasClientInstance())
        edu::captureClientInstance(self);

    if (!edu::rendering::isInstalled())
        edu::rendering::tryLazyInit();

    if (!g_tickHooked && self->getLocalPlayer()) {
        g_tickHooked = edu::features::hooks::LevelTick::install();
    }

    return o_update(self, isInitFinished);
}

bool install() {
    hat::scan_result result = hat::find_pattern(kSig, ".text");
    if (!result.get())
        return false;

    g_target = const_cast<std::byte*>(result.get());

    if (!Hooks::create(g_target, reinterpret_cast<void*>(&hk_update), &o_update))
        return false;
    if (!Hooks::enable(g_target))
        return false;

    return true;
}

void remove() {
    if (g_target) {
        MH_DisableHook(g_target);
        MH_RemoveHook(g_target);
        g_target = nullptr;
    }
    o_update = nullptr;
    g_tickHooked = false;
}

} // namespace edu::features::hooks::ClientInstanceUpdate
