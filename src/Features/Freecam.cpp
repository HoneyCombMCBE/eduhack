#include "Freecam.h"
#include "../Minecraft/Packet.h"
#include "../Minecraft/MovePlayerPacket.h"
#include "../Minecraft/PlayerAuthInputPacket.h"

namespace edu::features {

bool g_freecamEnabled = false;

static float s_lastPitch = 0.f;
static float s_lastYaw = 0.f;
static bool s_hasSavedRot = false;

void toggleFreecam() {
    g_freecamEnabled = !g_freecamEnabled;
    s_hasSavedRot = false;
}

void processFreecam(void* packet, bool& shouldSend) {
    if (!g_freecamEnabled || !packet) return;

    auto& pkt = *static_cast<Packet*>(packet);
    uint32_t id = static_cast<uint32_t>(pkt.getId());

    if (id == PACKET_ID_PLAYER_AUTH_INPUT) {
        auto& paip = *reinterpret_cast<PlayerAuthInputPacket*>(&pkt);

        if (!s_hasSavedRot) {
            s_lastPitch = paip.mRot().x;
            s_lastYaw = paip.mRot().y;
            s_hasSavedRot = true;
        }

        paip.mRot().x = s_lastPitch;
        paip.mRot().y = s_lastYaw;

        shouldSend = false;
    } else if (id == PACKET_ID_MOVE_PLAYER) {
        shouldSend = false;
    }
}

} // namespace edu::features
