#include "Notifications.h"
#include <imgui.h>
#include <vector>
#include <chrono>
#include <cmath>

namespace edu::gui::notifications {

struct Toast {
    std::string message;
    float spawnTime;
    float currentX;
};

static std::vector<Toast> g_toasts;

static float getTime() {
    static auto start = std::chrono::steady_clock::now();
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static float easeOutCubic(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
}

void notify(const std::string& message) {
    g_toasts.push_back({message, getTime(), 500.0f});
}

void render() {
    float now = getTime();
    auto& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;

    float yOffset = 24.0f;
    constexpr float toastH = 38.0f;
    constexpr float padding = 6.0f;
    constexpr float duration = 2.5f;
    constexpr float slideTime = 0.35f;
    constexpr float fadeTime = 0.6f;

    for (int i = (int)g_toasts.size() - 1; i >= 0; i--) {
        auto& t = g_toasts[i];
        float age = now - t.spawnTime;

        if (age > duration + fadeTime) {
            g_toasts.erase(g_toasts.begin() + i);
            continue;
        }

        float textW = ImGui::CalcTextSize(t.message.c_str()).x;
        float toastW = textW + 36.0f;

        float targetX = screenW - toastW - 20.0f;
        float startX = screenW + 10.0f;

        if (age < slideTime) {
            float p = easeOutCubic(age / slideTime);
            t.currentX = lerp(startX, targetX, p);
        } else if (age > duration) {
            float p = (age - duration) / fadeTime;
            t.currentX = lerp(targetX, screenW + 10.0f, p * p);
        } else {
            t.currentX = lerp(t.currentX, targetX, 0.2f);
        }

        float alpha = 1.0f;
        if (age > duration)
            alpha = 1.0f - ((age - duration) / fadeTime);
        alpha = std::max(0.0f, alpha);

        float posY = screenH - yOffset - toastH;
        ImDrawList* dl = ImGui::GetForegroundDrawList();

        float pulse = 0.5f + 0.5f * sinf(now * 2.5f);
        ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(
            ImVec4(0.05f, 0.10f, 0.12f, 0.94f * alpha));
        ImU32 borderCol = ImGui::ColorConvertFloat4ToU32(
            ImVec4(0.10f + 0.03f * pulse, 0.70f + 0.05f * pulse, 0.72f, 0.55f * alpha));
        ImU32 txtCol = ImGui::ColorConvertFloat4ToU32(
            ImVec4(0.85f, 0.98f, 0.96f, alpha));
        ImU32 accentLine = ImGui::ColorConvertFloat4ToU32(
            ImVec4(0.12f, 0.82f, 0.78f, 0.8f * alpha));

        ImVec2 p0(t.currentX, posY);
        ImVec2 p1(t.currentX + toastW, posY + toastH);

        dl->AddRectFilled(p0, p1, bgCol, 8.0f);
        dl->AddRect(p0, p1, borderCol, 8.0f, 0, 1.2f);

        dl->AddLine(ImVec2(p0.x + 3, p0.y + 6), ImVec2(p0.x + 3, p1.y - 6),
                    accentLine, 2.5f);

        dl->AddText(ImVec2(t.currentX + 18.0f, posY + (toastH - 14.0f) / 2.0f),
                    txtCol, t.message.c_str());

        yOffset += toastH + padding;
    }
}

} // namespace edu::gui::notifications
