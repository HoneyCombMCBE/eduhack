#include "Criticals.h"
#include "../Minecraft/PlayerAuthInputPacket.h"
#include "../Minecraft/Packet.h"

namespace edu::features {

bool g_criticalsEnabled = false;
int g_criticalsMode = 0; // 0 = Sentinel, 1 = Safe

void toggleCriticals() {
    g_criticalsEnabled = !g_criticalsEnabled;
}

// Exact Solstice Sentinel criticals state machine
enum class CritState { START, MID_AIR, MID_AIR2, LANDING, FINISHED };
static CritState s_state = CritState::START;
static float s_prevPosY = 0.f;
static bool s_hasPrevPos = false;

void processCriticals(void* rawPacket) {
    if (!g_criticalsEnabled) {
        s_state = CritState::START;
        s_hasPrevPos = false;
        return;
    }

    auto* packet = reinterpret_cast<Packet*>(rawPacket);
    if (static_cast<uint32_t>(packet->getId()) != PACKET_ID_PLAYER_AUTH_INPUT) return;

    auto* paip = reinterpret_cast<PlayerAuthInputPacket*>(rawPacket);
    float curY = paip->mPos().y;

    // Only apply when player is on flat ground (Y unchanged)
    if (s_hasPrevPos && curY == s_prevPosY) {

        bool sendJumping = (g_criticalsMode == 0); // Sentinel: send jump flags

        if (sendJumping) {
            paip->addInput(AuthInputAction::JUMP_DOWN);
            paip->addInput(AuthInputAction::JUMPING);
            paip->addInput(AuthInputAction::WANT_UP);
        }

        switch (s_state) {
            case CritState::START:
                paip->mPosDelta().y = -0.07840000092983246f;
                s_state = CritState::MID_AIR;
                break;

            case CritState::MID_AIR:
                paip->mPosDelta().y = -0.07840000092983246f;
                s_state = CritState::MID_AIR2;
                break;

            case CritState::MID_AIR2:
                if (sendJumping) {
                    paip->removeInput(AuthInputAction::JUMP_DOWN);
                    paip->removeInput(AuthInputAction::JUMPING);
                    paip->removeInput(AuthInputAction::WANT_UP);
                }
                paip->mPosDelta().y = -0.1552319973707199f;
                s_state = CritState::LANDING;
                break;

            case CritState::LANDING:
                paip->mPosDelta().y = -0.07840000092983246f;
                s_state = CritState::FINISHED;
                break;

            case CritState::FINISHED:
                s_state = CritState::START;
                break;
        }

        // Disable sprint for more consistent crits (exact Solstice)
        paip->removeInput(AuthInputAction::START_SPRINTING);
        paip->removeInput(AuthInputAction::SPRINTING);
        paip->removeInput(AuthInputAction::SPRINT_DOWN);
    }

    s_prevPosY = curY;
    s_hasPrevPos = true;
}

} // namespace edu::features
