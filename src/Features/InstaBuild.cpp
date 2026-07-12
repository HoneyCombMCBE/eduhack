#include "InstaBuild.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"
#include <cstring>

namespace edu::features {

bool g_instaBuildEnabled = false;

void toggleInstaBuild() { g_instaBuildEnabled = !g_instaBuildEnabled; }

static bool s_hadInstabuild = false;
static bool s_wasEnabled = false;

void tickInstaBuild(void* localPlayer) {
    if (!localPlayer) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* ac = ctx.tryGetComponent<AbilitiesComponent>();
    if (!ac) return;

    LayeredAbilities copy;
    std::memcpy(&copy, &ac->abilities, sizeof(LayeredAbilities));

    constexpr size_t kLayerStart = 8;
    constexpr size_t kLayerSize = 240;
    constexpr size_t kAbilitySize = 12;

    if (g_instaBuildEnabled) {
        if (!s_wasEnabled) {
            // Save original Instabuild state
            s_hadInstabuild = false;
            std::memcpy(&s_hadInstabuild, copy.data + kLayerStart + 2 * kAbilitySize, sizeof(bool));
        }
        bool val = true;
        for (int layer = 0; layer < 6; layer++) {
            size_t base = kLayerStart + layer * kLayerSize;
            std::memcpy(copy.data + base + 2 * kAbilitySize, &val, sizeof(bool));
        }
        s_wasEnabled = true;
    } else if (s_wasEnabled) {
        bool val = s_hadInstabuild;
        for (int layer = 0; layer < 6; layer++) {
            size_t base = kLayerStart + layer * kLayerSize;
            std::memcpy(copy.data + base + 2 * kAbilitySize, &val, sizeof(bool));
        }
        s_wasEnabled = false;
    }

    actor->setAbilities(copy);
}

} // namespace edu::features
