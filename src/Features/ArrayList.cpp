#include "ArrayList.h"
#include "../Client/ModuleManager.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace edu::features {

bool g_arrayListEnabled = false;

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

    auto& io = ImGui::GetIO();
    float dt = io.DeltaTime > 0 ? io.DeltaTime : 0.016f;
    float ff = ff_dt(dt);
    float sW = io.DisplaySize.x, sH = io.DisplaySize.y;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    float fontSize = sH * 0.018f;
    float rowH = sH * 0.028f;
    float rowGap = sH * 0.003f;
    float padX = sH * 0.012f;
    float padRight = 10.f;
    float padTop = 10.f;
    float rounding = sH * 0.005f;

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

    int slot = 0;
    for (auto& name : visible) {
        auto& e = g_entries[name];
        float targetY = padTop + slot * (rowH + rowGap);

        if (!e.initialized) {
            e.y = targetY;
            e.initialized = true;
        }

        fl(e.y, targetY, 0.15f * ff);

        ImVec2 ts = ImGui::CalcTextSize(name.c_str());
        float sc = fontSize / ImGui::GetFontSize();
        float textW = ts.x * sc;
        float rectW = textW + padX * 2.f;

        float slideOff = e.xSlide * (rectW + padRight + 20.f);
        float rx = sW - rectW - padRight + slideOff;
        float ry = e.y;

        float alpha = 1.f - e.xSlide;
        if (alpha < 0.01f) { slot++; continue; }

        float tx = rx + padX;
        float ty = ry + (rowH - ts.y * sc) / 2.f;

        dl->AddRectFilledMultiColor(
            ImVec2(rx - padX, ry), ImVec2(rx + rectW, ry + rowH),
            C(0, 0, 0, 0.0f), C(0, 0, 0, 0.12f * alpha),
            C(0, 0, 0, 0.12f * alpha), C(0, 0, 0, 0.0f));

        dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx + 1, ty + 1),
            C(0, 0, 0, 0.35f * alpha), name.c_str());

        int len = (int)name.size();
        int mid = len / 2;
        if (mid > 0) {
            std::string left = name.substr(0, mid);
            std::string right = name.substr(mid);
            float leftW = ImGui::CalcTextSize(left.c_str()).x * sc;
            dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx, ty),
                C(255, 90, 100, alpha), left.c_str());
            dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx + leftW, ty),
                C(255, 160, 160, alpha), right.c_str());
        } else {
            dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx, ty),
                C(255, 90, 100, alpha), name.c_str());
        }

        slot++;
    }
}

} // namespace edu::features
