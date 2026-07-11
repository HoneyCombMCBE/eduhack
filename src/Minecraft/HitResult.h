#pragma once
#include "Components.h"

enum class HitType : int {
    BLOCK = 0,
    ENTITY = 1,
    NONE = 2
};

struct BlockPos {
    int x, y, z;
};

struct HitResult {
    Vec3 mStartPos;          // 0x00
    Vec3 mRayDir;            // 0x0C
    HitType mType;           // 0x18
    unsigned char mFacing;   // 0x1C
    char _pad1[3];           // 0x1D
    BlockPos mBlock;         // 0x20
    Vec3 mPos;               // 0x2C
};
