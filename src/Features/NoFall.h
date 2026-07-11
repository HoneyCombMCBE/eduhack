#pragma once

namespace edu::features {

extern bool g_noFallEnabled;
void toggleNoFall();
void processNoFall(void* packet);

} // namespace edu::features
