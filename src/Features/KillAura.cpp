#include "KillAura.h"
#include "../Client/ClientStore.h"
#include "../Client/ClientInstance.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

#include <cmath>

namespace edu::features {

bool g_killAuraEnabled = false;

void toggleKillAura() { g_killAuraEnabled = !g_killAuraEnabled; }

static constexpr float kRange = 6.0f;
static constexpr int kDelay = 10;
static constexpr size_t kGameModeOffset = 0xA80;
static constexpr size_t kAttackIdx = 15;
static constexpr size_t kSwingIdx = 111;

using AttackFn = bool(__fastcall*)(void*, void*);
using SwingFn = bool(__fastcall*)(void*, int);

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

    void* closestActor = nullptr;
    float closestDist = kRange;

    for (auto ent : view) {
        auto& aoc = view.get<ActorOwnerComponent>(ent);
        if (!aoc.mActor || aoc.mActor == localPlayer) continue;

        auto& entSv = view.get<StateVectorComponent>(ent);

        float dx = entSv.pos.x - sv->pos.x;
        float dy = entSv.pos.y - sv->pos.y;
        float dz = entSv.pos.z - sv->pos.z;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (dist < 0.1f || dist >= closestDist) continue;

        closestDist = dist;
        closestActor = aoc.mActor;
    }

    if (!closestActor) return;

    auto* gmPtr = *reinterpret_cast<void**>((char*)localPlayer + kGameModeOffset);
    if (!gmPtr) return;

    auto playerVtable = *reinterpret_cast<uintptr_t**>(localPlayer);
    auto swing = reinterpret_cast<SwingFn>(playerVtable[kSwingIdx]);
    swing(localPlayer, 0);

    auto gmVtable = *reinterpret_cast<uintptr_t**>(gmPtr);
    auto attack = reinterpret_cast<AttackFn>(gmVtable[kAttackIdx]);
    attack(gmPtr, closestActor);

    delay = kDelay;
}

} // namespace edu::features
