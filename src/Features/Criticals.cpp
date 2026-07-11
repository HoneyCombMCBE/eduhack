#include "Criticals.h"
#include "../Minecraft/PlayerAuthInputPacket.h"
#include "../Minecraft/Packet.h"

namespace edu::features {

bool g_criticalsEnabled = false;

void toggleCriticals() {
    g_criticalsEnabled = !g_criticalsEnabled;
}

// Sentinel-style criticals state machine (from Solstice reference)
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

    // Only apply when on flat ground (Y position unchanged between ticks)
    if (s_hasPrevPos && curY == s_prevPosY) {

        // Inject jump flags
        paip->addInput(AuthInputAction::JUMP_DOWN);
        paip->addInput(AuthInputAction::JUMPING);
        paip->addInput(AuthInputAction::WANT_UP);

        switch (s_state) {
            case CritState::START:
                paip->mPosDelta().y = -0.0784000009f;
                s_state = CritState::MID_AIR;
                break;

            case CritState::MID_AIR:
                paip->mPosDelta().y += 0.2f;
                paip->mPosDelta().y = -0.0784000009f;
                s_state = CritState::MID_AIR2;
                break;

            case CritState::MID_AIR2:
                // Clear jump flags for landing phase
                paip->removeInput(AuthInputAction::JUMP_DOWN);
                paip->removeInput(AuthInputAction::JUMPING);
                paip->removeInput(AuthInputAction::WANT_UP);

                paip->mPosDelta().y += 0.1216f;
                paip->mPosDelta().y = -0.1552319974f;
                s_state = CritState::LANDING;
                break;

            case CritState::LANDING:
                paip->mPosDelta().y = -0.0784000009f;
                s_state = CritState::FINISHED;
                break;

            case CritState::FINISHED:
                s_state = CritState::START;
                break;
        }

        // Disable sprint to make crits more consistent
        paip->removeInput(AuthInputAction::START_SPRINTING);
        paip->removeInput(AuthInputAction::SPRINTING);
        paip->removeInput(AuthInputAction::SPRINT_DOWN);
        paip->removeInput(AuthInputAction::STOP_SPRINTING);
    }

    s_prevPosY = curY;
    s_hasPrevPos = true;
}

} // namespace edu::features
