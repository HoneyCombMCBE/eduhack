#pragma once

namespace edu::features {

extern bool g_killAuraEnabled;
extern int g_killAuraRange;
extern int g_killAuraDelay;
extern int g_killAuraMulti;
extern int g_killAuraTargets;
extern int g_killAuraFilterAlive;
void toggleKillAura();
void tickKillAura(void* localPlayer);

} // namespace edu::features
