#pragma once
#include <cstdint>
#include <string>
#include "VFunc.h"

namespace edu {

class ClientInstance {
public:
    void* getLocalPlayer() {
        return callVFunc<31, void*()>(this);
    }

    bool hasLevel() {
        return callVFunc<171, bool()>(this);
    }

    void* getLevel() {
        return callVFunc<169, void*()>(this);
    }

    std::string getScreenName() const {
        return callVFunc<246, std::string()>(this);
    }

    bool isInWorldNoMenus() const {
        return callVFunc<159, bool()>(this);
    }

    bool isInGame() {
        return getLocalPlayer() != nullptr;
    }

    void grabMouse() {
        callVFunc<310, void()>(this);
    }

    void releaseMouse() {
        callVFunc<311, void()>(this);
    }
};

} // namespace edu
