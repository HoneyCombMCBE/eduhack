#pragma once

namespace edu::features {

extern bool g_criticalsEnabled;
extern int g_criticalsMode; // 0 = Sentinel (full, for remote server), 1 = Safe (no jump flags, for hosting)
void toggleCriticals();
void processCriticals(void* packet);

} // namespace edu::features
