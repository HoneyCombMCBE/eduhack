#pragma once

namespace edu::features {

extern bool g_autoClickerEnabled;
extern int g_autoClickerDelay;
void toggleAutoClicker();
void tickAutoClicker(void* localPlayer);

} // namespace edu::features
