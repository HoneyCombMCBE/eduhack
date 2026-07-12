#pragma once
#include <vector>
#include <cstdint>

namespace edu::features {

extern bool g_blinkEnabled;
extern int g_blinkMode;    // 0=Normal, 1=Pulse
extern int g_blinkDelay;   // pulse delay in ticks (1-60)
void toggleBlink();
void processBlink(void* packet, bool& shouldSend);
void tickBlink();

} // namespace edu::features
