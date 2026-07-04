#include "KillAura.h"
#include "../Client/ClientStore.h"
#include "../Client/ClientInstance.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

#include <cmath>
#include <cstring>
#include <vector>
#include <memory>

namespace edu::features {

bool g_killAuraEnabled = false;

void toggleKillAura() { g_killAuraEnabled = !g_killAuraEnabled; }

static constexpr float kRange = 6.0f;
static constexpr int kDelay = 10;
static constexpr size_t kGameModeOffset = 0xAA0;
static constexpr size_t kAttackIdx = 15;
static constexpr size_t kSwingIdx = 111;
static constexpr size_t kGetEntitiesIdx = 224;

using AttackFn = bool(__fastcall*)(void*, void*);
using SwingFn = bool(__fastcall*)(void*, int);
using GetEntitiesFn = const void*(__fastcall*)(const void*);

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
    auto getEntities = reinterpret_cast<GetEntitiesFn>(levelVtable[kGetEntitiesIdx]);
    auto* vecPtr = getEntities(level);
    if (!vecPtr) return;

    struct SharedPtrLayout { void* ptr; void* ctrl; };

    auto* begin = *reinterpret_cast<SharedPtrLayout**>((char*)vecPtr);
    auto* end = *reinterpret_cast<SharedPtrLayout**>((char*)vecPtr + 8);
    if (!begin || !end) return;

    void* closestActor = nullptr;
    float closestDist = kRange;

    for (auto* it = begin; it < end; it++) {
        auto* entCtx = reinterpret_cast<EntityContext*>(it->ptr);
        if (!entCtx) continue;

        auto* entActor = reinterpret_cast<void*>((char*)entCtx - 0x8);
        if (entActor == localPlayer) continue;

        auto* entSv = entCtx->tryGetComponent<StateVectorComponent>();
        if (!entSv) continue;

        float dx = entSv->pos.x - sv->pos.x;
        float dy = entSv->pos.y - sv->pos.y;
        float dz = entSv->pos.z - sv->pos.z;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (dist < closestDist) {
            closestDist = dist;
            closestActor = entActor;
        }
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
