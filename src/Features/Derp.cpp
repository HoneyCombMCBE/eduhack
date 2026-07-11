#include "Derp.h"
#include "../Minecraft/PlayerAuthInputPacket.h"
#include "../Minecraft/Packet.h"
#include <cstdlib>

namespace edu::features {

bool g_derpEnabled = false;

void toggleDerp() {
    g_derpEnabled = !g_derpEnabled;
}

void processDerp(void* rawPacket) {
    if (!g_derpEnabled) return;

    auto* packet = reinterpret_cast<Packet*>(rawPacket);
    if (static_cast<uint32_t>(packet->getId()) != PACKET_ID_PLAYER_AUTH_INPUT) return;

    auto* paip = reinterpret_cast<PlayerAuthInputPacket*>(rawPacket);

    // Randomize head rotation server-side
    float randYaw = static_cast<float>(rand() % 360) - 180.0f;
    float randPitch = static_cast<float>(rand() % 180) - 90.0f;

    paip->mRot().x = randPitch;
    paip->mRot().y = randYaw;
    paip->mYHeadRot() = randYaw;
}

} // namespace edu::features
