#pragma once

namespace edu::features {

extern bool g_noFallEnabled;
extern int g_noFallMode; // 0 = Sentinel, 1 = BDS, 2 = Horion
void toggleNoFall();
void processNoFall(void* packet);

} // namespace edu::features
