#include "Notifications.h"
#include <imgui.h>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>

// 1:1 port of flarial NotifyHeartbeat (Engine.cpp:1734-1798)

namespace edu::gui::notifications {

struct Notification {
    float currentPos = 0;
    float currentPosY = 0;
    std::string text;
    bool finished = false;
    bool arrived = false;
    float width = 0;
    bool firstTime = true;
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
};

static std::vector<Notification> g_notifications;

// flarial lerp (Engine.cpp:1975) — NO clamp, ceil rounded
static void fl(float& a, float b, float t) {
    float v = a + (b - a) * t;
    a = std::ceilf(v * 1000.f) / 1000.f;
}

static float rnd(float r, float sH) { return r * sH / 1920.f; }

static ImU32 col(int r, int g, int b, float a = 1.f) {
    return IM_COL32(r, g, b, (int)(a * 255.f));
}

void notify(const std::string& message) {
    Notification n;
    n.text = message;
    n.currentPosY = ImGui::GetIO().DisplaySize.y;
    g_notifications.push_back(n);
}

void clear() {
    g_notifications.clear();
}

void render() {
    auto& io = ImGui::GetIO();
    float sH = io.DisplaySize.y;
    float sW = io.DisplaySize.x;

    // flarial frameFactor = 60/fps clamped <=1, floored to 2 decimals
    float dt = io.DeltaTime > 0 ? io.DeltaTime : 0.016f;
    float frameFactor = dt * 60.f;
    if (frameFactor > 1.f) frameFactor = 1.f;
    frameFactor = std::floor(frameFactor * 100.f) / 100.f;

    // flarial: RoundingConstraint(20,20) → round.x = 20 * sH / 1920
    float roundX = rnd(20, sH);
    // flarial: height = RelativeConstraint(0.035, "height", true)
    float height = sH * 0.035f;
    // flarial: fontSize = RelativeConstraint(0.128, "height", true)
    // This goes through FlarialTextWithFont which scales: targetFontSize = fontSize * 0.18
    float fontSize = sH * 0.128f;
    float imguiFontSize = fontSize * 0.18f;

    // primary1: #ff233a (flarial exact red)
    ImU32 accentCol = col(255, 35, 58);

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    int i = 0;
    for (auto& n : g_notifications) {
        // flarial: posyModif = -((height + RelativeConstraint(0.01, "height", true)) * i)
        float posyModif = -((height + sH * 0.01f) * i);

        if (n.firstTime) {
            // flarial: measure text with FlarialTextWithFont at transparent color
            // width = measured text width + RelativeConstraint(0.0345, "height", true)
            ImVec2 ts = ImGui::CalcTextSize(n.text.c_str());
            float scale = imguiFontSize / ImGui::GetFontSize();
            float textW = ts.x * scale;
            n.width = textW + sH * 0.0345f;
            // flarial: CenterConstraint(width, 0).x = (screenWidth - width) / 2
            n.currentPos = (sW - n.width) / 2.f;
            n.firstTime = false;
        }

        if (!n.finished) {
            if (!n.arrived) {
                // flarial: draw FIRST, then lerp (draw at pre-lerp position)
                dl->AddRectFilled(
                    ImVec2(n.currentPos, n.currentPosY + posyModif),
                    ImVec2(n.currentPos + n.width, n.currentPosY + posyModif + height),
                    accentCol, roundX);

                // text centered in rect
                {
                    ImVec2 ts = ImGui::CalcTextSize(n.text.c_str());
                    float sc = imguiFontSize / ImGui::GetFontSize();
                    float tx = n.currentPos + (n.width - ts.x * sc) / 2.f;
                    float ty = n.currentPosY + posyModif + (height - ts.y * sc) / 2.f;
                    dl->AddText(ImGui::GetFont(), imguiFontSize, ImVec2(tx, ty),
                        col(255, 255, 255), n.text.c_str());
                }

                // flarial: lerp toward PercentageConstraint(0.1, "bottom", true) = sH * 0.9
                fl(n.currentPosY, sH * 0.9f, frameFactor * 0.067f);

                // flarial: arrive when <= PercentageConstraint(0.08, "bottom", true) = sH * 0.92
                if (n.currentPosY <= sH * 0.92f) {
                    n.arrived = true;
                    n.time = std::chrono::steady_clock::now();
                }

                i++;
            } else {
                auto current = std::chrono::steady_clock::now();
                auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(current - n.time);

                // flarial: draw FIRST, then lerp
                dl->AddRectFilled(
                    ImVec2(n.currentPos, n.currentPosY + posyModif),
                    ImVec2(n.currentPos + n.width, n.currentPosY + posyModif + height),
                    accentCol, roundX);

                {
                    ImVec2 ts = ImGui::CalcTextSize(n.text.c_str());
                    float sc = imguiFontSize / ImGui::GetFontSize();
                    float tx = n.currentPos + (n.width - ts.x * sc) / 2.f;
                    float ty = n.currentPosY + posyModif + (height - ts.y * sc) / 2.f;
                    dl->AddText(ImGui::GetFont(), imguiFontSize, ImVec2(tx, ty),
                        col(255, 255, 255), n.text.c_str());
                }

                // flarial: after 5000ms, lerp out
                if (timeDiff.count() > 5000) {
                    fl(n.currentPosY, sH + 500, frameFactor * 0.052f);
                    if (n.currentPosY >= sH) { n.finished = true; }
                }

                i++;
            }
        } else {
            g_notifications.erase(g_notifications.begin() + (&n - &g_notifications[0]));
            break;
        }
    }
}

} // namespace edu::gui::notifications
