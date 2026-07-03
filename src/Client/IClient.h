#pragma once
#include "ClientInstance.h"
#include "ClientStore.h"

namespace edu {

class IClient {
public:
    static ClientInstance* clientInstance() { return getClientInstance(); }
    static bool isReady() { return hasClientInstance(); }
};

} // namespace edu
