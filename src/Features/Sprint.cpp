#include "Sprint.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

namespace edu::features {

bool g_sprintEnabled = false;

void toggleSprint() { g_sprintEnabled = !g_sprintEnabled; }

static constexpr uint32_t kSprintDown = 1 << 8;
static constexpr uint16_t kSprinting = 1 << 1;

void tickSprint(void* localPlayer) {
    if (!localPlayer || !g_sprintEnabled) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* mic = ctx.tryGetComponent<MoveInputComponent>();
    if (!mic) return;

    mic->mInputState.mFlagValues |= kSprintDown;
    mic->mRawInputState.mFlagValues |= kSprintDown;
    mic->mFlagValues |= kSprinting;
}

} // namespace edu::features
