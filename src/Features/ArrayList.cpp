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

        float rx = sW - totalTextW - padRight - accentW - fontSize * 0.4f + e.xSlide * (totalTextW + padRight + 30.f);
        dl->AddRectFilledMultiColor(
            ImVec2(rx - fontSize * 0.3f, e.y), ImVec2(sW, e.y + rowH),
            C(200, 120, 40, bgAlpha * alpha),
            C(200, 50, 60, bgAlpha * alpha),
            C(180, 40, 50, bgAlpha * alpha),
            C(220, 140, 50, bgAlpha * alpha));

        slot0++;
    }

    int slot = 0;
    int totalVisible = (int)visible.size();
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

        float t_slot = totalVisible > 1 ? (float)slot / (totalVisible - 1) : 0.f;
        int ar = (int)(255 - t_slot * 40);
        int ag = (int)(70 + t_slot * 30);
        int ab = (int)(80 + t_slot * 30);

        float accentGap = fontSize * 0.25f;
        dl->AddRectFilled(
            ImVec2(tx - accentW - accentGap, e.y),
            ImVec2(tx - accentGap, e.y + rowH),
            C(ar, ag, ab, alpha));

        float cx = tx;
        int len = (int)name.size();
        char ch[2] = {0, 0};
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
