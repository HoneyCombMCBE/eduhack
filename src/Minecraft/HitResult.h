#pragma once
#include "Components.h"
#include <cstdint>

namespace edu {

enum class HitResultType : uint8_t {
    Block  = 0,
    Entity = 1,
    None   = 2,
};

struct HitResult {
    Vec3    mStartPos;      // 0x00
    Vec3    mRayDir;        // 0x0C
    HitResultType mType;    // 0x18
    uint8_t mFacing;        // 0x1C
    char    _pad1[3];       // 0x1D
    int32_t mBlockX;        // 0x20
    int32_t mBlockY;        // 0x24
    int32_t mBlockZ;        // 0x28
    Vec3    mPos;           // 0x2C
    uint64_t mEntity;       // 0x38  (WeakEntityRef - raw handle)
    char    _pad2[8];       // 0x40
    AABB    mEntityAABB;    // 0x50
    bool    mIsHitLiquid;   // 0x68
    uint8_t mLiquidFacing;  // 0x69
    char    _pad3[2];       // 0x6A
    int32_t mLiquidX;       // 0x6C
    int32_t mLiquidY;       // 0x70
    int32_t mLiquidZ;       // 0x74
    Vec3    mLiquidPos;     // 0x78
    bool    mIndirectHit;   // 0x84
    char    _pad4[3];       // 0x85

    bool isBlock()  const { return mType == HitResultType::Block; }
    bool isEntity() const { return mType == HitResultType::Entity; }
    bool isNone()   const { return mType == HitResultType::None; }

    void setBlock(int x, int y, int z, uint8_t face) {
        mBlockX = x; mBlockY = y; mBlockZ = z;
        mFacing = face;
        mType = HitResultType::Block;
    }
};

struct HitResultWrapper {
    HitResult mHitResult;       // 0x000
    HitResult mLiquidHitResult; // 0x088
    // 0x110 onwards: subscriptions (don't touch)

    HitResult* operator->() { return &mHitResult; }
    HitResult& operator*()  { return mHitResult; }
};

} // namespace edu
