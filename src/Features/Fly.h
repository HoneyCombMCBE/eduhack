#pragma once

namespace edu::features {

extern bool g_flyEnabled;
bool isFlyEnabled();
bool toggleFly();
void tickFly(void* localPlayer);

} // namespace edu::features
