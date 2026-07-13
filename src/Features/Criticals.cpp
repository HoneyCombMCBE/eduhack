#include "Criticals.h"
#include "../Minecraft/PlayerAuthInputPacket.h"
#include "../Minecraft/Packet.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

namespace edu::features {

bool g_criticalsEnabled = false;
int g_criticalsMode = 0; // 0 = Sentinel, 1 = Safe
int g_criticalsRange = 5;
int g_criticalsFilterAlive = 1;

void toggleCriticals() {
    g_criticalsEnabled = !g_criticalsEnabled;
}

// Exact Solstice Sentinel criticals state machine
enum class CritState { START, MID_AIR, MID_AIR2, LANDING, FINISHED };
static CritState s_state = CritState::START;
static float s_prevPosY = 0.f;
static bool s_hasPrevPos = false;

void processCriticals(void* rawPacket, void* localPlayer) {
    if (!g_criticalsEnabled || !localPlayer) {
        s_state = CritState::START;
        s_hasPrevPos = false;
        return;
    }

    auto* packet = reinterpret_cast<Packet*>(rawPacket);
    if (static_cast<uint32_t>(packet->getId()) != PACKET_ID_PLAYER_AUTH_INPUT) return;

    auto* paip = reinterpret_cast<PlayerAuthInputPacket*>(rawPacket);
    float curY = paip->mPos().y;

    float nearEntityDist = 999.f;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* sv = ctx.tryGetComponent<StateVectorComponent>();
    if (!sv) return;

    auto& reg = ctx.enttRegistry;
    auto view = reg.view<ActorOwnerComponent, StateVectorComponent>();

    std::vector<Actor*> targets;
    float closestDistSq = static_cast<float>(g_criticalsRange * g_criticalsRange);

    for (auto ent : view) {
        auto& aoc = view.get<ActorOwnerComponent>(ent);
        if (!aoc.mActor || aoc.mActor.get() == actor) continue;

        auto* targetActor = aoc.mActor.get();

        if (g_criticalsFilterAlive == 1) {
            if (targetActor->getHealth() <= 0.f) continue;
        }

        auto& entSv = view.get<StateVectorComponent>(ent);
        float dx = entSv.pos.x - sv->pos.x;
        float dy = entSv.pos.y - sv->pos.y;
        float dz = entSv.pos.z - sv->pos.z;
        float distSq = dx*dx + dy*dy + dz*dz;

        if (distSq < closestDistSq) {
            nearEntityDist = std::sqrt(distSq);
            targets.push_back(targetActor);
        }
    }

    if (targets.empty()) return;

    if (s_hasPrevPos && curY == s_prevPosY && nearEntityDist < g_criticalsRange) {

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
