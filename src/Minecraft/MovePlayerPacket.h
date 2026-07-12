#pragma once
#include "Components.h"
#include <cstdint>

namespace edu {

static constexpr uint32_t PACKET_ID_MOVE_PLAYER = 0x13;

// MovePlayerPacket layout
// Packet base is 0x30 bytes, fields follow after
struct MovePlayerPacket {
    int64_t& mPlayerID()      { return *reinterpret_cast<int64_t*>(reinterpret_cast<char*>(this) + 0x30); }
    Vec3&    mPos()           { return *reinterpret_cast<Vec3*>(reinterpret_cast<char*>(this) + 0x38); }
    Vec2&    mRot()           { return *reinterpret_cast<Vec2*>(reinterpret_cast<char*>(this) + 0x44); }
    float&   mYHeadRot()      { return *reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x4C); }
    uint8_t& mResetPosition() { return *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(this) + 0x50); }
    bool&    mOnGround()      { return *reinterpret_cast<bool*>(reinterpret_cast<char*>(this) + 0x51); }
    int64_t& mRidingID()      { return *reinterpret_cast<int64_t*>(reinterpret_cast<char*>(this) + 0x58); }
};

} // namespace edu
