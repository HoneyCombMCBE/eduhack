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

void tickFly(void* localPlayer) {
    if (!localPlayer || !g_flyEnabled) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* ac = ctx.tryGetComponent<AbilitiesComponent>();
    if (!ac) return;

    LayeredAbilities copy;
    std::memcpy(&copy, &ac->abilities, sizeof(LayeredAbilities));

    bool val = true;
    for (int layer = 0; layer < 6; layer++) {
        size_t base = kLayerStart + layer * kLayerSize;
        std::memcpy(copy.data + base + kMayFlyIdx * kAbilitySize, &val, sizeof(bool));
        std::memcpy(copy.data + base + kFlyingIdx * kAbilitySize, &val, sizeof(bool));
    }

    auto vtable = *reinterpret_cast<void***>(localPlayer);
    auto setAbilities = reinterpret_cast<SetAbilitiesFn>(vtable[241]);
    setAbilities(localPlayer, &copy);
}

} // namespace edu::features
