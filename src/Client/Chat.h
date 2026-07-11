#pragma once
#include "ClientStore.h"
#include "../Minecraft/Actor.h"
#include <string>

namespace edu {

inline void sendChatMessage(const std::string& message) {
    auto* ci = getClientInstance();
    if (!ci) return;
    auto* player = reinterpret_cast<Actor*>(ci->getLocalPlayer());
    if (!player) return;
    player->displayClientMessage(message);
}

inline void logChat(const std::string& message) {
    static const char red[] = {'\xC2', '\xA7', 'c', '\0'};
    static const char reset[] = {'\xC2', '\xA7', 'r', '\0'};
    sendChatMessage(std::string(red) + "[heheboi]" + reset + " " + message);
}

} // namespace edu
