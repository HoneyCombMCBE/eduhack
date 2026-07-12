#include "AntiHunger.h"
#include "../Minecraft/MovePlayerPacket.h"
#include "../Minecraft/Packet.h"

namespace edu::features {

void toggleAntiHunger() {
    g_antiHungerEnabled = !g_antiHungerEnabled;
}

void processAntiHunger(void* rawPacket) {
    if (!g_antiHungerEnabled) return;
    if (!rawPacket) return;

    auto* packet = reinterpret_cast<Packet*>(rawPacket);
    uint32_t id = static_cast<uint32_t>(packet->getId());
    if (id == PACKET_ID_MOVE_PLAYER) {
        auto* mpp = reinterpret_cast<MovePlayerPacket*>(rawPacket);
        mpp->mOnGround() = false;
    }
}

} // namespace edu::features
