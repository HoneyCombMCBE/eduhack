#pragma once

namespace edu::features {

extern bool g_criticalsEnabled;
void toggleCriticals();
void processCriticals(void* packet);

} // namespace edu::features
