#pragma once

namespace edu::features {

inline bool g_antiHungerEnabled = false;

void toggleAntiHunger();
void processAntiHunger(void* packet);

} // namespace edu::features
