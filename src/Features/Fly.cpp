#include "Fly.h"
#include "../Core/Log.h"
#include "../Input/KeyInput.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

#include <windows.h>

namespace edu::features {

static bool g_enabled = false;

bool isFlyEnabled() { return g_enabled; }

bool toggleFly() {
    g_enabled = !g_enabled;
    LOG_INFO("Fly %s", g_enabled ? "ON" : "OFF");
    return g_enabled;
}

static float g_hoverY = 0.0f;
static bool g_hasHoverY = false;

void tickFly(void* localPlayer) {
    if (!localPlayer) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* svc = ctx.tryGetComponent<StateVectorComponent>();
    if (!svc) return;

    if (!g_enabled) {
        g_hasHoverY = false;
        return;
    }

    constexpr float speed = 0.6f;

    if (input::isHeld(VK_SPACE)) {
        svc->velocity.y = speed;
    } else if (input::isHeld(VK_SHIFT)) {
        svc->velocity.y = -speed;
    } else {
        svc->velocity.y = 0.0f;
    }
}

} // namespace edu::features
