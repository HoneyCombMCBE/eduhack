#include "KillAura.h"
#include "../Client/ClientStore.h"
#include "../Client/ClientInstance.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

#include <cmath>
#include <vector>

namespace edu::features {

bool g_killAuraEnabled = false;

void toggleKillAura() { g_killAuraEnabled = !g_killAuraEnabled; }

static constexpr float kRange = 6.0f;
static constexpr int kDelay = 10;
static constexpr size_t kGameModeOffset = 0xA80;
static constexpr size_t kAttackIdx = 15;
static constexpr size_t kSwingIdx = 111;
static constexpr size_t kGetRuntimeActorListIdx = 317;

using AttackFn = bool(__fastcall*)(void*, void*);
using SwingFn = bool(__fastcall*)(void*, int);
using GetActorListFn = const std::vector<void*>&(__fastcall*)(const void*);

void tickKillAura(void* localPlayer) {
    if (!localPlayer || !g_killAuraEnabled) return;

    static int delay = 0;
    if (delay > 0) { delay--; return; }

    auto* ci = edu::getClientInstance();
    if (!ci) return;
    void* level = ci->getLevel();
    if (!level) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& ctx = actor->getEntity();
    auto* sv = ctx.tryGetComponent<StateVectorComponent>();
    if (!sv) return;

    auto levelVtable = *reinterpret_cast<uintptr_t**>(level);
    auto getActorList = reinterpret_cast<GetActorListFn>(levelVtable[kGetRuntimeActorListIdx]);
    auto& actors = getActorList(level);

    void* closestActor = nullptr;
    float closestDist = kRange;

    for (auto* entPtr : actors) {
        if (!entPtr || entPtr == localPlayer) continue;

        auto* ent = reinterpret_cast<Actor*>(entPtr);
        auto& entCtx = ent->getEntity();
        auto* entSv = entCtx.tryGetComponent<StateVectorComponent>();
        if (!entSv) continue;

        float dx = entSv->pos.x - sv->pos.x;
        float dy = entSv->pos.y - sv->pos.y;
        float dz = entSv->pos.z - sv->pos.z;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (dist < 0.1f) continue;
        if (dist >= closestDist) continue;

        closestDist = dist;
        closestActor = entPtr;
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
