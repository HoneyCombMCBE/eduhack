#include "NoFall.h"
#include "../Minecraft/PlayerAuthInputPacket.h"
#include "../Minecraft/Packet.h"

namespace edu::features {

bool g_noFallEnabled = false;
int g_noFallMode = 0; // 0 = Sentinel, 1 = BDS

void toggleNoFall() {
    g_noFallEnabled = !g_noFallEnabled;
}

static bool s_alt = false;

void processNoFall(void* rawPacket) {
    if (!g_noFallEnabled) return;

    auto* packet = reinterpret_cast<Packet*>(rawPacket);
    if (static_cast<uint32_t>(packet->getId()) != PACKET_ID_PLAYER_AUTH_INPUT) return;

    auto* paip = reinterpret_cast<PlayerAuthInputPacket*>(rawPacket);

    if (g_noFallMode == 0) {
        // Sentinel: alternate ticks, clamp mPosDelta.y (exact Solstice)
        s_alt = !s_alt;
        if (s_alt) return;
        paip->mPosDelta().y = -0.0784000015f;
    }
    else if (g_noFallMode == 1) {
        // BDS: spoof gliding flag + zero velocity (exact Solstice)
        paip->addInput(AuthInputAction::START_GLIDING);
        paip->removeInput(AuthInputAction::STOP_GLIDING);
        paip->mPosDelta().x = 0.f;
        paip->mPosDelta().y = 0.f;
        paip->mPosDelta().z = 0.f;
    }
}

} // namespace edu::features
