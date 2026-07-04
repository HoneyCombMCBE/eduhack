#include "Fly.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

#include <cstring>

namespace edu::features {

bool g_flyEnabled = false;

bool isFlyEnabled() { return g_flyEnabled; }

bool toggleFly() {
    g_flyEnabled = !g_flyEnabled;
    return g_flyEnabled;
}

static constexpr size_t kLayerStart  = 8;
static constexpr size_t kLayerSize   = 240;
static constexpr size_t kAbilitySize = 12;
static constexpr size_t kFlyingIdx   = 9;
static constexpr size_t kMayFlyIdx   = 10;

using SetAbilitiesFn = void(__fastcall*)(void*, const LayeredAbilities*);

static bool g_wasEnabled = false;
static bool g_hadMayFly = false;

static void applyFly(void* localPlayer, bool enable) {
    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* ac = ctx.tryGetComponent<AbilitiesComponent>();
    if (!ac) return;

    LayeredAbilities copy;
    std::memcpy(&copy, &ac->abilities, sizeof(LayeredAbilities));

    bool val = enable;
    for (int layer = 0; layer < 6; layer++) {
        size_t base = kLayerStart + layer * kLayerSize;
        std::memcpy(copy.data + base + kMayFlyIdx * kAbilitySize, &val, sizeof(bool));
        std::memcpy(copy.data + base + kFlyingIdx * kAbilitySize, &val, sizeof(bool));
    }

    auto vtable = *reinterpret_cast<void***>(localPlayer);
    auto setAbilities = reinterpret_cast<SetAbilitiesFn>(vtable[241]);
    setAbilities(localPlayer, &copy);
}

void tickFly(void* localPlayer) {
    if (!localPlayer) return;

    if (g_flyEnabled) {
        if (!g_wasEnabled) {
            auto* actor = reinterpret_cast<Actor*>(localPlayer);
            auto& ctx = actor->getEntity();
            auto* mac = ctx.tryGetComponent<MovementAbilitiesComponent>();
            g_hadMayFly = mac && mac->mayFly;
        }
        applyFly(localPlayer, true);
        g_wasEnabled = true;
    } else if (g_wasEnabled) {
        if (!g_hadMayFly)
            applyFly(localPlayer, false);
        g_wasEnabled = false;
    }
}

} // namespace edu::features
