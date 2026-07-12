#pragma once

namespace edu::features {

extern bool g_freecamEnabled;
void toggleFreecam();
void processFreecam(void* packet, bool& shouldSend);

} // namespace edu::features
