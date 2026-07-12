#pragma once

namespace edu::features {

extern bool g_flyEnabled;
extern int g_flyMode;     // 0=Creative, 1=Motion, 2=Elytra
extern int g_flySpeed;
bool isFlyEnabled();
bool toggleFly();
void tickFly(void* localPlayer);
void processFlyPacket(void* packet);

} // namespace edu::features
