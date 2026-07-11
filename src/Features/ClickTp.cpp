#include "ClickTp.h"
#include "../Client/ClientInstance.h"
#include "../Client/ClientStore.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Level.h"

namespace edu::features {

bool g_clickTpEnabled = false;

void toggleClickTp() {
    g_clickTpEnabled = !g_clickTpEnabled;
    if (g_clickTpEnabled) {
        auto* ci = getClientInstance();
        if (ci) {
            auto* player = reinterpret_cast<Actor*>(ci->getLocalPlayer());
            auto* level = reinterpret_cast<Level*>(ci->getLevel());
            if (player && level) {
                auto& hitResult = level->getHitResult();
                if (hitResult.mType == HitType::BLOCK) {
                    Vec3 newPos = {
                        static_cast<float>(hitResult.mBlock.x) + 0.5f,
                        static_cast<float>(hitResult.mBlock.y) + 1.01f + 1.8f,
                        static_cast<float>(hitResult.mBlock.z) + 0.5f
                    };
                    player->setPosition(newPos);
                }
            }
        }
        g_clickTpEnabled = false;
    }
}

void tickClickTp(void* localPlayer) {
    // ClickTp runs instantly on toggle, tick is a no-op
}

} // namespace edu::features
