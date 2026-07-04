#include "ArrayList.h"
#include "../Client/ModuleManager.h"
#include "../Client/ClientStore.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace edu::features {

bool g_arrayListEnabled = false;
int g_arrayListShadowAlpha = 25;
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
    float rowH = fontSize * 1.15f;
    float rowGap = sH * 0.001f;
    float padRight = sH * 0.008f;
    float padTop = sH * 0.008f;
    float bgAlpha = g_arrayListShadowAlpha / 100.f;

    std::vector<std::string> active;
    for (auto& m : edu::getModules()) {
        if (m.name == "ArrayList") continue;
        if (m.enabled && *m.enabled)
            active.push_back(m.name);
    }
    std::sort(active.begin(), active.end());

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
    std::sort(visible.begin(), visible.end());

    float maxTextW = 0.f;
    float sc = fontSize / ImGui::GetFontSize();
    for (auto& name : visible) {
        float w = ImGui::CalcTextSize(name.c_str()).x * sc;
        if (w > maxTextW) maxTextW = w;
    }

    float bgW = maxTextW + padRight * 2.f;
    float bgH = visible.size() * (rowH + rowGap) - rowGap + padTop * 0.5f;
    float bgX = sW - bgW;
    float bgY = padTop * 0.5f;

    if (!visible.empty() && bgAlpha > 0.01f) {
        dl->AddRectFilledMultiColor(
            ImVec2(bgX - bgW * 0.3f, bgY),
            ImVec2(bgX + bgW, bgY + bgH),
            C(0, 0, 0, 0.0f),
            C(40, 10, 12, bgAlpha),
            C(40, 10, 12, bgAlpha),
            C(0, 0, 0, 0.0f));
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

        float slideOff = e.xSlide * (textW + padRight + 30.f);
        float tx = sW - textW - padRight + slideOff;
        float ty = e.y + (rowH - ImGui::CalcTextSize(name.c_str()).y * sc) / 2.f;

        float alpha = 1.f - e.xSlide;
        if (alpha < 0.01f) { slot++; continue; }

        float cx = tx;
        int len = (int)name.size();
        char ch[2] = {0, 0};
        float bold = fontSize * 0.03f;
        for (int i = 0; i < len; i++) {
            float t = len > 1 ? (float)i / (len - 1) : 0.f;
            int r = (int)(255 - t * 35);
            int g = (int)(130 + t * 40);
            int b = (int)(80 + t * 25);
            ImU32 col = C(r, g, b, alpha);

            ch[0] = name[i];
            dl->AddText(ImGui::GetFont(), fontSize, ImVec2(cx + bold, ty), col, ch);
            dl->AddText(ImGui::GetFont(), fontSize, ImVec2(cx, ty), col, ch);
            cx += ImGui::CalcTextSize(ch).x * sc + bold;
        }

        slot++;
    }
}

} // namespace edu::features
