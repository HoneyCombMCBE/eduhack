#pragma once
#include "HitResult.h"
#include <cstdint>

namespace edu {

// Minimal Level layout — only offsets we need
class Level {
public:
    // vtable is at offset 0 (inherited from ILevel etc.)

    // +0x1E0: gsl::not_null<UniqueOwnerPointer<HitResultWrapper>>
    // This is a smart pointer — need to dereference to get the actual HitResultWrapper
    HitResultWrapper& getHitResultWrapper() {
        auto* ptr = *reinterpret_cast<HitResultWrapper**>(reinterpret_cast<char*>(this) + 0x1E0);
        return *ptr;
    }

    HitResult& getHitResult() {
        return getHitResultWrapper().mHitResult;
    }

    // Convenience: get crosshair block position
    void getCrosshairBlock(int& x, int& y, int& z, uint8_t& face) {
        auto& hr = getHitResult();
        x = hr.mBlockX;
        y = hr.mBlockY;
        z = hr.mBlockZ;
        face = hr.mFacing;
    }

    bool isLookingAtBlock() {
        return getHitResult().isBlock();
    }

    bool isLookingAtEntity() {
        return getHitResult().isEntity();
    }
};

} // namespace edu
