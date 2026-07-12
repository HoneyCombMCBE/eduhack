#pragma once

namespace edu::features {

extern bool g_crasherEnabled;
void toggleCrasher();
void processCrasher(void* packet, bool& shouldSend);

} // namespace edu::features
