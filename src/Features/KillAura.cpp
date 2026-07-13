#include "KillAura.h"
#include "../Client/ClientStore.h"
#include "../Client/ClientInstance.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/GameMode.h"
#include "../Minecraft/Components.h"
#include "../Client/Chat.h"
#include <sstream>

#include <cmath>

namespace edu::features {

bool g_killAuraEnabled = false;
int g_killAuraRange = 20;
int g_killAuraDelay = 2;
int g_killAuraMulti = 0;
int g_killAuraTargets = 0; // 0 = All, 1 = Players, 2 = Mobs
int g_killAuraFilterAlive = 1;

void toggleKillAura() { g_killAuraEnabled = !g_killAuraEnabled; }

void tickKillAura(void* localPlayer) {
    if (!localPlayer || !g_killAuraEnabled) return;

    static int delay = 0;
    if (delay > 0) { delay--; return; }

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* sv = ctx.tryGetComponent<StateVectorComponent>();
    if (!sv) return;

    auto& reg = ctx.enttRegistry;
    auto view = reg.view<ActorOwnerComponent, StateVectorComponent>();

    std::vector<Actor*> targets;
    Actor* closestActorPtr = nullptr;
    float rangeLimit = static_cast<float>(g_killAuraRange);
    float closestDistSq = rangeLimit * rangeLimit;

    for (auto ent : view) {
        auto& aoc = view.get<ActorOwnerComponent>(ent);
        if (!aoc.mActor || aoc.mActor.get() == actor) continue;

        auto* targetActor = aoc.mActor.get();
        auto& entCtx = targetActor->getEntity();

        // 1. Skip dead entities
        if (g_killAuraFilterAlive == 1) {
            if (targetActor->getHealth() <= 0.f) continue;
        }

        // 2. Filter targets: 0 = All, 1 = Players Only (has AbilitiesComponent), 2 = Mobs Only (no AbilitiesComponent)
        bool isTargetPlayer = entCtx.hasComponent<AbilitiesComponent>();
        if (g_killAuraTargets == 1 && !isTargetPlayer) continue;
        if (g_killAuraTargets == 2 && isTargetPlayer) continue;

        // 3. Distance check
        auto& entSv = view.get<StateVectorComponent>(ent);
        float dx = entSv.pos.x - sv->pos.x;
        float dy = entSv.pos.y - sv->pos.y;
        float dz = entSv.pos.z - sv->pos.z;
        float distSq = dx*dx + dy*dy + dz*dz;

        if (distSq < 0.01f || distSq >= rangeLimit * rangeLimit) continue;

        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestActorPtr = targetActor;
        }
        targets.push_back(targetActor);
    }

    if (targets.empty()) return;

    auto& gm = actor->getGameMode();
    if (!gm) return;

    actor->swing();
    
    if (g_killAuraMulti == 1) {
        for (auto* target : targets) {
            gm->attack(target);
        }
    } else {
        if (closestActorPtr) {
            gm->attack(closestActorPtr);
        }
    }

    delay = g_killAuraDelay;
}

} // namespace edu::features
