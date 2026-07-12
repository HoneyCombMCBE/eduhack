#include "Fly.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"
#include "../Minecraft/PlayerAuthInputPacket.h"
#include "../Minecraft/Packet.h"
#include <cmath>
#include <cstring>

namespace edu::features {

bool g_flyEnabled = false;
int g_flyMode = 0;
int g_flySpeed = 1;

bool isFlyEnabled() { return g_flyEnabled; }

bool toggleFly() {
    g_flyEnabled = !g_flyEnabled;
    return g_flyEnabled;
}

// Creative mode ability patching
static constexpr size_t kLayerStart  = 8;
static constexpr size_t kLayerSize   = 240;
static constexpr size_t kAbilitySize = 12;
static constexpr size_t kFlyingIdx   = 9;
static constexpr size_t kMayFlyIdx   = 10;

static bool g_wasEnabled = false;
static bool g_hadMayFly = false;

static void applyFly(Actor* actor, bool enable) {
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

    actor->setAbilities(copy);
}

void tickFly(void* localPlayer) {
    if (!localPlayer) return;
    auto* actor = reinterpret_cast<Actor*>(localPlayer);

    if (g_flyEnabled && g_flyMode == 0) {
        if (!g_wasEnabled) {
            auto& ctx = actor->getEntity();
            auto* mac = ctx.tryGetComponent<MovementAbilitiesComponent>();
            g_hadMayFly = mac && mac->mayFly;
        }
        applyFly(actor, true);
        g_wasEnabled = true;
    } else if (g_wasEnabled && g_flyMode == 0) {
        if (!g_hadMayFly)
            applyFly(actor, false);
        g_wasEnabled = false;
    }

    // Motion mode: set velocity each tick
    if (g_flyEnabled && g_flyMode == 1) {
        auto& ctx = actor->getEntity();
        auto* sv = ctx.tryGetComponent<StateVectorComponent>();
        if (sv) {
            sv->posDelta.x = 0.f;
            sv->posDelta.y = 0.f;
            sv->posDelta.z = 0.f;
        }
    }
}

void processFlyPacket(void* packet) {
    if (!g_flyEnabled || !packet) return;

    auto& pkt = *static_cast<Packet*>(packet);
    uint32_t id = static_cast<uint32_t>(pkt.getId());
    if (id != PACKET_ID_PLAYER_AUTH_INPUT) return;

    auto& paip = *reinterpret_cast<PlayerAuthInputPacket*>(&pkt);

    if (g_flyMode == 1) {
        // Motion mode: add glide flags so server applies elytra physics
        paip.addInput(AuthInputAction::START_GLIDING);
        paip.removeInput(AuthInputAction::STOP_GLIDING);

        // Zero out position delta so server doesn't see impossible movement
        paip.mPosDelta() = { 0.f, 0.f, 0.f };
    } else if (g_flyMode == 2) {
        // Elytra toggle mode: alternate glide flags each tick
        static bool s_alt = false;
        s_alt = !s_alt;

        if (s_alt) {
            paip.addInput(AuthInputAction::START_GLIDING);
            paip.addInput(AuthInputAction::ASCEND);
            paip.addInput(AuthInputAction::WANT_UP);
            paip.removeInput(AuthInputAction::STOP_GLIDING);
            paip.removeInput(AuthInputAction::DESCEND);
            paip.removeInput(AuthInputAction::WANT_DOWN);
            paip.removeInput(AuthInputAction::SNEAKING);
        } else {
            paip.addInput(AuthInputAction::STOP_GLIDING);
            paip.addInput(AuthInputAction::JUMPING);
            paip.addInput(AuthInputAction::START_JUMPING);
            paip.addInput(AuthInputAction::JUMP_DOWN);
            paip.removeInput(AuthInputAction::START_GLIDING);
            paip.removeInput(AuthInputAction::ASCEND);
            paip.removeInput(AuthInputAction::WANT_UP);
        }
    }
}

} // namespace edu::features
