#pragma once
#include "../Minecraft/Components.h"
#include <cstdint>

namespace edu {

// AuthInputAction bit flags for PlayerAuthInputPacket::mInputData
enum class AuthInputAction : uint64_t {
    NONE             = 0,
    ASCEND           = 1ULL << 0,
    DESCEND          = 1ULL << 1,
    JUMP_DOWN        = 1ULL << 3,
    SPRINT_DOWN      = 1ULL << 4,
    CHANGE_HEIGHT    = 1ULL << 5,
    JUMPING          = 1ULL << 6,
    SNEAKING         = 1ULL << 8,
    SNEAK_DOWN       = 1ULL << 9,
    UP               = 1ULL << 10,
    DOWN             = 1ULL << 11,
    LEFT             = 1ULL << 12,
    RIGHT            = 1ULL << 13,
    UP_LEFT          = 1ULL << 14,
    UP_RIGHT         = 1ULL << 15,
    WANT_UP          = 1ULL << 16,
    WANT_DOWN        = 1ULL << 17,
    SPRINTING        = 1ULL << 20,
    ASCEND_BLOCK     = 1ULL << 21,
    DESCEND_BLOCK    = 1ULL << 22,
    START_SPRINTING  = 1ULL << 25,
    STOP_SPRINTING   = 1ULL << 26,
    START_SNEAKING   = 1ULL << 27,
    STOP_SNEAKING    = 1ULL << 28,
    START_JUMPING    = 1ULL << 31,
    START_GLIDING    = 1ULL << 32,
    STOP_GLIDING     = 1ULL << 33,
    START_FLYING     = 1ULL << 42,
    STOP_FLYING      = 1ULL << 43,
};

inline AuthInputAction operator|(AuthInputAction a, AuthInputAction b) {
    return static_cast<AuthInputAction>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}
inline AuthInputAction operator&(AuthInputAction a, AuthInputAction b) {
    return static_cast<AuthInputAction>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}
inline AuthInputAction operator~(AuthInputAction a) {
    return static_cast<AuthInputAction>(~static_cast<uint64_t>(a));
}
inline void operator|=(AuthInputAction& a, AuthInputAction b) { a = a | b; }
inline void operator&=(AuthInputAction& a, AuthInputAction b) { a = a & b; }

// PlayerAuthInputPacket layout — offset-based accessors
// Real game Packet base is 0x30 bytes, fields follow after
struct PlayerAuthInputPacket {
    Vec2& mRot()              { return *reinterpret_cast<Vec2*>(reinterpret_cast<char*>(this) + 0x30); }
    Vec3& mPos()              { return *reinterpret_cast<Vec3*>(reinterpret_cast<char*>(this) + 0x38); }
    float& mYHeadRot()        { return *reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x44); }
    Vec3& mPosDelta()         { return *reinterpret_cast<Vec3*>(reinterpret_cast<char*>(this) + 0x48); }
    Vec2& mMove()             { return *reinterpret_cast<Vec2*>(reinterpret_cast<char*>(this) + 0x64); }
    Vec2& mInteractRotation() { return *reinterpret_cast<Vec2*>(reinterpret_cast<char*>(this) + 0x6C); }

    // mInputData is std::bitset<65> (16 bytes), first 64 bits as uint64_t
    AuthInputAction& mInputData() {
        return *reinterpret_cast<AuthInputAction*>(reinterpret_cast<char*>(this) + 0x88);
    }

    void addInput(AuthInputAction flag) { mInputData() |= flag; }
    void removeInput(AuthInputAction flag) { mInputData() &= ~flag; }
    bool hasInput(AuthInputAction flag) {
        return (mInputData() & flag) == flag;
    }
};

static constexpr uint32_t PACKET_ID_PLAYER_AUTH_INPUT = 144;

} // namespace edu
