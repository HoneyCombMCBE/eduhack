#include "NoFall.h"
#include "../Minecraft/PlayerAuthInputPacket.h"
#include "../Minecraft/Packet.h"

namespace edu::features {

bool g_noFallEnabled = false;

void toggleNoFall() {
    g_noFallEnabled = !g_noFallEnabled;
}

void processNoFall(void* rawPacket) {
    if (!g_noFallEnabled) return;

    auto* packet = reinterpret_cast<Packet*>(rawPacket);
    if (static_cast<uint32_t>(packet->getId()) != PACKET_ID_PLAYER_AUTH_INPUT) return;

    auto* paip = reinterpret_cast<PlayerAuthInputPacket*>(rawPacket);

    // If falling (negative Y delta), zero it out to prevent fall damage
    if (paip->mPosDelta().y < -0.08f) {
        paip->mPosDelta().y = -0.0784000009f; // Vanilla gravity tick value
    }
}

} // namespace edu::features
