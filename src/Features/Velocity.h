#pragma once

namespace edu::features {

extern bool g_velocityEnabled;
extern int g_velocityMode;   // 0=Off, 1=Reverse, 2=Zero
extern int g_velocityHorizontal; // 0-100 percent
extern int g_velocityVertical;   // 0-100 percent
void toggleVelocity();
void tickVelocity(void* localPlayer);

} // namespace edu::features
