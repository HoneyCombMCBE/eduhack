#pragma once
#include <windows.h>

namespace edu::input {

inline bool isJustPressed(int vk) {
    static bool prev[256]{};
    bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool result = down && !prev[vk];
    prev[vk] = down;
    return result;
}

inline bool isHeld(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

} // namespace edu::input
