#pragma once
#include "ClientStore.h"
#include <string>
#include <optional>

namespace edu {

static constexpr size_t kDisplayClientMessageIdx = 199;

inline void sendChatMessage(const std::string& message) {
    auto* ci = getClientInstance();
    if (!ci) return;
    void* player = ci->getLocalPlayer();
    if (!player) return;
    auto vtable = *reinterpret_cast<uintptr_t**>(player);
    auto fn = reinterpret_cast<void(__fastcall*)(void*, const std::string&, std::optional<std::string>)>(
        vtable[kDisplayClientMessageIdx]);
    fn(player, message, std::nullopt);
}

inline void logChat(const std::string& message) {
    static const char red[] = {'\xC2', '\xA7', 'c', '\0'};
    static const char reset[] = {'\xC2', '\xA7', 'r', '\0'};
    sendChatMessage(std::string(red) + "[heheboi]" + reset + " " + message);
}

} // namespace edu
