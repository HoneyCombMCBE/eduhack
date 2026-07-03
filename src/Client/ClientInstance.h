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
        return callVFunc<172, bool()>(this);
    }

    void* getLevel() {
        return callVFunc<170, void*()>(this);
    }

    std::string getScreenName() const {
        return callVFunc<247, std::string()>(this);
    }

    bool isInWorldNoMenus() const {
        return callVFunc<160, bool()>(this);
    }

    bool isInGame() {
        return getLocalPlayer() != nullptr;
    }

    static constexpr size_t kGrabMouseIdx = 311;
    static constexpr size_t kReleaseMouseIdx = 312;

    void grabMouse() {
        callVFunc<kGrabMouseIdx, void()>(this);
    }
    void releaseMouse() {
        callVFunc<kReleaseMouseIdx, void()>(this);
    }
};

} // namespace edu
