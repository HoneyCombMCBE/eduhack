#pragma once
#include <cstdint>
#include <string>
#include "VFunc.h"
#include "../Minecraft/Options.h"
#include "../Minecraft/PacketSender.h"

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
        return callVFunc<171, void*()>(this);
    }

    std::string getScreenName() const {
        return callVFunc<247, std::string()>(this);
    }

    bool isInWorldNoMenus() const {
        return callVFunc<160, bool()>(this);
    }

    void* getGameRenderer() const {
        return callVFunc<187, void*()>(this);
    }

    void* getLevelRenderer() const {
        return callVFunc<188, void*()>(this);
    }

    PacketSender& getPacketSender() {
        return callVFunc<294, PacketSender&()>(this);
    }

    void* getCamera() const {
        return callVFunc<205, void*()>(this);
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
    Options* getOptions() {
        return callVFunc<177, Options*()>(this);
    }
};

} // namespace edu
