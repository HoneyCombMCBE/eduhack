#include "AutoClicker.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/GameMode.h"
#include "../Client/ClientInstance.h"
#include "../Client/ClientStore.h"
#include "../Minecraft/Level.h"
#include <windows.h>

namespace edu::features {

bool g_autoClickerEnabled = false;
int g_autoClickerDelay = 2;

void toggleAutoClicker() { g_autoClickerEnabled = !g_autoClickerEnabled; }

static int s_acTick = 0;

void tickAutoClicker(void* localPlayer) {
    if (!g_autoClickerEnabled || !localPlayer) return;

    bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (!leftDown && !rightDown) {
        s_acTick = 0;
        return;
    }

    s_acTick++;
    if (s_acTick <= g_autoClickerDelay) return;
    s_acTick = 0;

    auto* ci = edu::getClientInstance();
    if (!ci) return;

    auto* actor = reinterpret_cast<Actor*>(localPlayer);
    auto& gmPtr = actor->getGameMode();
    if (!gmPtr) return;

    actor->swing();

    if (leftDown) {
        auto* level = ci->getLevel();
        if (level && level->isLookingAtEntity()) {
            auto& hr = level->getHitResult();
            if (hr.mEntity) {
                gmPtr->attack(reinterpret_cast<Actor*>(hr.mEntity));
            }
        }
    }

    if (rightDown) {
        auto* level = ci->getLevel();
        if (level && level->isLookingAtBlock()) {
            auto& hr = level->getHitResult();
            gmPtr->buildBlock(BlockPos(hr.mBlockX, hr.mBlockY, hr.mBlockZ), hr.mFacing);
        }
    }
}

} // namespace edu::features
