#pragma once
#include "ClientInstance.h"
#include <atomic>

namespace edu {

extern ClientInstance* g_ClientInstance;

void captureClientInstance(ClientInstance* instance);
ClientInstance* getClientInstance();
bool hasClientInstance();

inline bool isOnHudScreen() {
    auto* ci = getClientInstance();
    if (!ci || !ci->getLocalPlayer()) return false;
    std::string s = ci->getScreenName();
    return s == "hud_screen" || s == "pause_screen" || s == "f3_screen" || s == "zoom_screen";
}

} // namespace edu
