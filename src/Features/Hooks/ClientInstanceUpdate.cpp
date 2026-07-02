#include "ClientInstanceUpdate.h"

#include "../../Core/Log.h"
#include "../../Memory/Hooks.h"
#include "../../Client/ClientInstance.h"
#include "../../Client/ClientStore.h"
#include "../../Input/KeyInput.h"
#include "../Fly.h"

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

static bool g_loggedPlayer = false;

static bool __fastcall hk_update(ClientInstance* self, bool isInitFinished) {
    if (!edu::hasClientInstance()) {
        LOG_INFO("ClientInstanceUpdate: captured self=%p", self);
        edu::captureClientInstance(self);
    }

    void* player = self->getLocalPlayer();

    if (!g_loggedPlayer && player) {
        LOG_INFO("getLocalPlayer() = %p", player);
        g_loggedPlayer = true;
    }

    if (player) {
        if (edu::input::isJustPressed('G'))
            edu::features::toggleFly();

        edu::features::tickFly(player);
    }

    return o_update(self, isInitFinished);
}

bool install() {
    LOG_INFO("ClientInstanceUpdate: scanning for signature...");
    hat::scan_result result = hat::find_pattern(kSig, ".text");
    if (!result.get()) {
        LOG_ERROR("ClientInstanceUpdate: signature not found");
        return false;
    }
    g_target = const_cast<std::byte*>(result.get());
    LOG_INFO("ClientInstanceUpdate: target = %p", g_target);

    if (!Hooks::create(g_target,
                       reinterpret_cast<void*>(&hk_update),
                       &o_update)) {
        LOG_ERROR("ClientInstanceUpdate: MH_CreateHook failed");
        return false;
    }
    if (!Hooks::enable(g_target)) {
        LOG_ERROR("ClientInstanceUpdate: MH_EnableHook failed");
        return false;
    }

    LOG_INFO("ClientInstanceUpdate: hook installed & enabled "
             "(trampoline=%p)", reinterpret_cast<void*>(o_update));
    return true;
}

void remove() {
    if (g_target) {
        MH_DisableHook(g_target);
        MH_RemoveHook(g_target);
        LOG_INFO("ClientInstanceUpdate: hook removed");
    }
}

} // namespace edu::features::hooks::ClientInstanceUpdate
