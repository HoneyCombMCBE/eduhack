#pragma once

namespace edu::features {

extern bool g_criticalsEnabled;
extern int g_criticalsMode;
extern int g_criticalsRange;
void toggleCriticals();
void processCriticals(void* packet, void* localPlayer);

} // namespace edu::features
