#include "NoSlow.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

namespace edu::features {

bool g_noSlowEnabled = false;

void toggleNoSlow() { g_noSlowEnabled = !g_noSlowEnabled; }

void tickNoSlow(void* localPlayer) {
    if (!g_noSlowEnabled || !localPlayer) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* comp = ctx.tryGetComponent<BlockMovementSlowdownMultiplierComponent>();
    if (!comp) return;

    comp->mMultiplier = {0.f, 0.f, 0.f};
}

} // namespace edu::features
