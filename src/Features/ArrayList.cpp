#include "ArrayList.h"
#include "../Client/ModuleManager.h"
#include "../Client/ClientStore.h"
#include "../Client/Theme.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace edu::features {

bool g_arrayListEnabled = false;
int g_arrayListShadowAlpha = 75;
int g_arrayListFontSize = 32;

void toggleArrayList() { g_arrayListEnabled = !g_arrayListEnabled; }

static void fl(float& a, float b, float t) {
    a = std::ceilf((a + (b - a) * t) * 1000.f) / 1000.f;
}

static float ff_dt(float dt) {
    float v = dt * 60.f;
    if (v > 1.f) v = 1.f;
    return std::floor(v * 100.f) / 100.f;
}

static ImU32 C(int r, int g, int b, float a = 1.f) {
    return IM_COL32(r, g, b, (int)(a * 255.f));
}

struct EntryAnim {
    float y;
    float xSlide;
    bool initialized;
};

static std::map<std::string, EntryAnim> g_entries;

void renderArrayList() {
    if (!g_arrayListEnabled) return;
    if (!edu::isOnHudScreen()) return;

    auto& io = ImGui::GetIO();
    float dt = io.DeltaTime > 0 ? io.DeltaTime : 0.016f;
    float ff = ff_dt(dt);
    float sW = io.DisplaySize.x, sH = io.DisplaySize.y;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    float fontSize = sH * (g_arrayListFontSize / 1000.f);
    float sc = fontSize / ImGui::GetFontSize();
    float rowH = fontSize * 1.15f;
    float rowGap = sH * 0.001f;
    float padRight = sH * 0.008f;
    float padTop = sH * 0.008f;
    float bgAlpha = g_arrayListShadowAlpha / 100.f;
    float accentW = sH * 0.003f;

    std::vector<std::string> active;
    for (auto& m : edu::getModules()) {
        if (m.name == "ArrayList") continue;
        if (m.enabled && *m.enabled)
            active.push_back(m.name);
    }

    std::sort(active.begin(), active.end(), [&](const std::string& a, const std::string& b) {
        float wa = ImGui::CalcTextSize(a.c_str()).x;
        float wb = ImGui::CalcTextSize(b.c_str()).x;
        if (wa != wb) return wa > wb;
        return a < b;
    });

    for (auto& name : active) {
        if (g_entries.find(name) == g_entries.end())
            g_entries[name] = {0.f, 1.f, false};
    }

    for (auto& [name, e] : g_entries) {
        bool isActive = std::find(active.begin(), active.end(), name) != active.end();
        float targetX = isActive ? 0.f : 1.f;
        fl(e.xSlide, targetX, 0.12f * ff);
    }

    for (auto it = g_entries.begin(); it != g_entries.end(); ) {
        if (it->second.xSlide > 0.99f &&
            std::find(active.begin(), active.end(), it->first) == active.end())
            it = g_entries.erase(it);
        else
            ++it;
    }

    std::vector<std::string> visible;
    for (auto& name : active)
        visible.push_back(name);
    for (auto& [name, e] : g_entries) {
        if (std::find(active.begin(), active.end(), name) == active.end())
            visible.push_back(name);
    }
    std::sort(visible.begin(), visible.end(), [&](const std::string& a, const std::string& b) {
        float wa = ImGui::CalcTextSize(a.c_str()).x;
        float wb = ImGui::CalcTextSize(b.c_str()).x;
        if (wa != wb) return wa > wb;
        return a < b;
    });

    float maxTextW = 0.f;
    for (auto& name : visible) {
        float w = ImGui::CalcTextSize(name.c_str()).x * sc;
        if (w > maxTextW) maxTextW = w;
    }

    int slot0 = 0;
    for (auto& name : visible) {
        auto& e = g_entries[name];
        float targetY = padTop + slot0 * (rowH + rowGap);
        if (!e.initialized) { e.y = targetY; e.initialized = true; }
        fl(e.y, targetY, 0.15f * ff);

        float textW = ImGui::CalcTextSize(name.c_str()).x * sc;
        float bold = fontSize * 0.03f;
        float totalTextW = textW + bold * (int)name.size();

        float alpha = 1.f - e.xSlide;
        if (alpha < 0.01f) { slot0++; continue; }

        float rx = sW - totalTextW - padRight + e.xSlide * (totalTextW + padRight + 30.f);
        dl->AddRectFilled(
            ImVec2(rx - fontSize * 0.15f, e.y), ImVec2(sW, e.y + rowH),
            C(20, 20, 20, bgAlpha * alpha));

        // Draw vertical accent bar at the right screen edge using theme color
        ImColor themeCol = edu::getThemeColor(e.y * 2.f);
        dl->AddRectFilled(
            ImVec2(sW - accentW, e.y), ImVec2(sW, e.y + rowH),
            IM_COL32(themeCol.Value.x * 255.f, themeCol.Value.y * 255.f, themeCol.Value.z * 255.f, alpha * 255.f));

        slot0++;
    }

    int slot = 0;
    for (auto& name : visible) {
        auto& e = g_entries[name];
        float targetY = padTop + slot * (rowH + rowGap);

        if (!e.initialized) {
            e.y = targetY;
            e.initialized = true;
        }

        fl(e.y, targetY, 0.15f * ff);

        float textW = ImGui::CalcTextSize(name.c_str()).x * sc;
        float bold = fontSize * 0.03f;
        float totalTextW = textW + bold * (int)name.size();

        float slideOff = e.xSlide * (totalTextW + padRight + 30.f);
        float tx = sW - totalTextW - padRight + slideOff;
        float ty = e.y + (rowH - ImGui::CalcTextSize(name.c_str()).y * sc) / 2.f;

        float alpha = 1.f - e.xSlide;
        if (alpha < 0.01f) { slot++; continue; }

        float cx = tx;
        int len = (int)name.size();
        char ch[2] = {0, 0};
        for (int i = 0; i < len; i++) {
            float indexFactor = e.y * 2.f + (static_cast<float>(i) / (len > 1 ? (len - 1) : 1)) * 100.f;
            ImColor themeCol = edu::getThemeColor(indexFactor);
            ImU32 col = IM_COL32(themeCol.Value.x * 255.f, themeCol.Value.y * 255.f, themeCol.Value.z * 255.f, alpha * 255.f);

            ch[0] = name[i];
            dl->AddText(ImGui::GetFont(), fontSize, ImVec2(cx + bold, ty), col, ch);
            dl->AddText(ImGui::GetFont(), fontSize, ImVec2(cx, ty), col, ch);
            cx += ImGui::CalcTextSize(ch).x * sc + bold;
        }

        slot++;
    }
}

} // namespace edu::features
