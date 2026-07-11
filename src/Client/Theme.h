#pragma once
#include <imgui.h>
#include <vector>
#include <cmath>
#include <windows.h>

namespace edu {

extern int g_activeTheme;

inline ImColor getThemeColor(float index) {
    if (g_activeTheme == 0) { // Rainbow
        static uint64_t start_time = GetTickCount64();
        uint64_t elapsed = GetTickCount64() - start_time;
        float hue = static_cast<float>((elapsed + static_cast<uint64_t>(index * 2.f)) % 6000) / 6000.0f;
        return ImColor::HSV(hue, 0.75f, 0.9f);
    }
    else if (g_activeTheme == 1) { // Sunset
        static std::vector<ImColor> colors = {
            ImColor(213, 32, 0, 255),
            ImColor(239, 118, 39, 255),
            ImColor(255, 154, 86, 255),
            ImColor(209, 98, 164, 255),
            ImColor(181, 86, 144, 255)
        };
        static uint64_t start_time = GetTickCount64();
        uint64_t elapsed = GetTickCount64() - start_time;
        
        float time = 6000.0f;
        float angle = fmodf(static_cast<float>(elapsed + static_cast<int>(index * 4.f)), time);
        float segmentTime = time / colors.size();

        int segmentIndex = static_cast<int>(angle / segmentTime) % colors.size();
        float segmentIndexFloat = (angle / segmentTime) - static_cast<int>(angle / segmentTime);

        ImColor startColor = colors[segmentIndex];
        ImColor endColor = colors[(segmentIndex + 1) % colors.size()];
        
        float r = startColor.Value.x + (endColor.Value.x - startColor.Value.x) * segmentIndexFloat;
        float g = startColor.Value.y + (endColor.Value.y - startColor.Value.y) * segmentIndexFloat;
        float b = startColor.Value.z + (endColor.Value.z - startColor.Value.z) * segmentIndexFloat;
        return ImColor(r, g, b, 1.0f);
    }
    else { // Orange
        float t = index / 20.f;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        int r = static_cast<int>(255 - t * 35);
        int g = static_cast<int>(130 + t * 40);
        int b = static_cast<int>(80 + t * 25);
        return ImColor(r, g, b, 255);
    }
}

} // namespace edu
