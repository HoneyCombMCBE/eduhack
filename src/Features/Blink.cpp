#include "Blink.h"
#include "../Minecraft/Packet.h"
#include "../Minecraft/MovePlayerPacket.h"
#include "../Minecraft/PlayerAuthInputPacket.h"

namespace edu::features {

bool g_blinkEnabled = false;
int g_blinkMode = 0;    // 0=Normal, 1=Pulse
int g_blinkDelay = 10;

static int s_pulseTick = 0;

void toggleBlink() {
    g_blinkEnabled = !g_blinkEnabled;
    s_pulseTick = 0;
}

void processBlink(void* packet, bool& shouldSend) {
    if (!g_blinkEnabled || !packet) return;

    auto& pkt = *static_cast<Packet*>(packet);
    uint32_t id = static_cast<uint32_t>(pkt.getId());

    if (id != PACKET_ID_MOVE_PLAYER && id != PACKET_ID_PLAYER_AUTH_INPUT) return;

    if (g_blinkMode == 0) {
        shouldSend = false;
    } else {
        if (s_pulseTick % g_blinkDelay != 0) {
            shouldSend = false;
        }
    }
}

void tickBlink() {
    if (!g_blinkEnabled) {
        s_pulseTick = 0;
        return;
    }
    s_pulseTick++;
}

} // namespace edu::features
