#pragma once
#include "ClientInstance.h"
#include <atomic>

namespace edu {

extern ClientInstance* g_ClientInstance;

void captureClientInstance(ClientInstance* instance);
ClientInstance* getClientInstance();
bool hasClientInstance();

} // namespace edu
