#pragma once

namespace edu {

class FloatOption {
public:
    char pad_0000[16]; // 0x0
    float mMinimum;    // 0x10
    float mMaximum;    // 0x14
    float mValue;      // 0x18
    float mDefaultValue; // 0x1C
};

class Options {
public:
    // Memory layout offsets match the Solstice reference
    // CLASS_FIELD(FloatOption*, mGfxFieldOfView, 0x1A0);
    // CLASS_FIELD(FloatOption*, mGfxGamma, 0x1B8);
    char pad_0000[0x1A0];
    FloatOption* mGfxFieldOfView;
    char pad_01A8[16];
    FloatOption* mGfxGamma;
};

} // namespace edu
