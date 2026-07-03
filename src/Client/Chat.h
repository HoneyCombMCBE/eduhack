#pragma once
#include "ClientStore.h"
#include <string>
#include <optional>

namespace edu {

// LocalPlayer::displayClientMessage — VFT 198
inline void sendChatMessage(const std::string& message) {
    auto* ci = getClientInstance();
    if (!ci) return;
    void* player = ci->getLocalPlayer();
    if (!player) return;

    using Fn = void(__fastcall*)(void*, const std::string&, std::optional<std::string>);
    auto** vtable = *reinterpret_cast<void***>(player);
    auto fn = reinterpret_cast<Fn>(vtable[198]);
    fn(player, message, std::nullopt);
}

inline void logChat(const std::string& message) {
    sendChatMessage("[heheboi] " + message);
}

} // namespace edu
