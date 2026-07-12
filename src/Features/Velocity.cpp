#include "Velocity.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"
#include <cmath>

namespace edu::features {

bool g_velocityEnabled = false;
int g_velocityMode = 2;
int g_velocityHorizontal = 0;
int g_velocityVertical = 0;

void toggleVelocity() { g_velocityEnabled = !g_velocityEnabled; }

void tickVelocity(void* localPlayer) {
    if (!g_velocityEnabled || !localPlayer) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* sv = ctx.tryGetComponent<StateVectorComponent>();
    if (!sv) return;

    float h = g_velocityHorizontal / 100.f;
    float v = g_velocityVertical / 100.f;

    if (g_velocityMode == 2) {
        sv->posDelta.x *= h;
        sv->posDelta.y *= v;
        sv->posDelta.z *= h;
    } else if (g_velocityMode == 1) {
        sv->posDelta.x = -sv->posDelta.x * h;
        sv->posDelta.z = -sv->posDelta.z * h;
        sv->posDelta.y = sv->posDelta.y * v;
    }
}

} // namespace edu::features
