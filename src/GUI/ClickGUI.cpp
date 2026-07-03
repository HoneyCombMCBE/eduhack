#include "ClickGUI.h"
#include "Notifications.h"
#include "../Client/ModuleManager.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <map>

namespace edu::gui {

static bool g_open = false;

void toggle() { g_open = !g_open; }
bool isOpen() { return g_open; }

static void fl(float& a, float b, float t) {
    a = std::ceilf((a + (b - a) * t) * 1000.f) / 1000.f;
}
static float ff_dt(float dt) {
    float v = dt * 60.f;
    if (v > 1.f) v = 1.f;
    return std::floor(v * 100.f) / 100.f;
}
static float rnd(float r, float sH) { return r * sH / 1920.f; }
static ImU32 C(int r, int g, int b, float a = 1.f) {
    return IM_COL32(r, g, b, (int)(a * 255.f));
}

static void ShadowRect(ImDrawList* dl, float x, float y, float w, float h,
                        float rounding, int layers) {
    if (layers < 1) return;
    for (int i = layers; i >= 1; i--) {
        float e = (float)i * 1.5f;
        float a = 0.5f * (1.f - (float)i / (layers + 1.f));
        a *= a;
        dl->AddRectFilled(ImVec2(x-e,y-e), ImVec2(x+w+e,y+h+e), C(0,0,0,a), rounding);
    }
}

struct CategoryState {
    float x = -1, y = -1;
    bool dragging = false;
    float dragOffX = 0, dragOffY = 0;
    bool posInit = false;
};

static std::map<std::string, CategoryState> g_cats;
static float g_animAlpha = 0.f;

void applyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = s.ChildRounding = s.FrameRounding = 0;
    s.WindowPadding = s.FramePadding = s.ItemSpacing = ImVec2(0,0);
    s.WindowBorderSize = s.ChildBorderSize = 0;
    s.ScrollbarSize = 5; s.ScrollbarRounding = 3;
    auto* cl = s.Colors;
    cl[ImGuiCol_WindowBg] = cl[ImGuiCol_ChildBg] = ImVec4(0,0,0,0);
    cl[ImGuiCol_ScrollbarBg]   = ImVec4(0,0,0,0.08f);
    cl[ImGuiCol_ScrollbarGrab] = ImVec4(1.f,0.42f,0.42f,0.35f);
}

void render() {
    auto& io = ImGui::GetIO();
    float dt = io.DeltaTime > 0 ? io.DeltaTime : 0.016f;
    float ff = ff_dt(dt);
    float sW = io.DisplaySize.x, sH = io.DisplaySize.y;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    fl(g_animAlpha, g_open ? 1.f : 0.f, 0.15f * ff);
    if (g_animAlpha < 0.01f) return;

    auto cats = edu::getCategories();
    auto& mods = edu::getModules();

    // sizing
    float panelW   = sH * 0.095f;
    float headerH   = sH * 0.028f;
    float rowH      = sH * 0.022f;
    float rowGap    = sH * 0.001f;
    float padY      = sH * 0.005f;
    float panelRnd  = rnd(14, sH);
    float fontSize  = sH * 0.013f;
    float headerFs  = sH * 0.015f;

    ImVec2 mp = io.MousePos;
    bool mouseDown = ImGui::IsMouseDown(0);
    bool mouseClicked = ImGui::IsMouseClicked(0);

    // init category positions: spread evenly
    float totalW = (float)cats.size() * panelW + (float)(cats.size() - 1) * sH * 0.012f;
    float startX = (sW - totalW) / 2.f;
    float startY = sH * 0.15f;

    int catIdx = 0;
    for (auto& catName : cats) {
        auto& cs = g_cats[catName];
        if (!cs.posInit) {
            cs.x = startX + catIdx * (panelW + sH * 0.012f);
            cs.y = startY;
            cs.posInit = true;
        }

        // collect modules in this category
        std::vector<ModuleInfo*> catMods;
        for (auto& m : mods)
            if (m.category == catName) catMods.push_back(&m);

        int modCount = (int)catMods.size();
        float panelH = headerH + padY + modCount * (rowH + rowGap) + padY;

        float px = cs.x, py = cs.y;

        // drag handling: header area
        bool inHeader = (mp.x >= px && mp.x <= px + panelW && mp.y >= py && mp.y <= py + headerH);
        if (inHeader && mouseClicked && !cs.dragging) {
            bool anyOtherDragging = false;
            for (auto& [k, v] : g_cats) if (v.dragging) anyOtherDragging = true;
            if (!anyOtherDragging) {
                cs.dragging = true;
                cs.dragOffX = mp.x - px;
                cs.dragOffY = mp.y - py;
            }
        }
        if (cs.dragging) {
            if (mouseDown) {
                cs.x = mp.x - cs.dragOffX;
                cs.y = mp.y - cs.dragOffY;
                px = cs.x; py = cs.y;
            } else {
                cs.dragging = false;
            }
        }

        // shadow
        ShadowRect(dl, px, py, panelW, panelH, panelRnd, 6);

        // panel bg: dark #1a1a1e
        dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + panelH),
            C(26, 26, 30, 0.94f * g_animAlpha), panelRnd);

        // header bg: slight gradient accent
        dl->AddRectFilledMultiColor(
            ImVec2(px, py), ImVec2(px + panelW, py + headerH),
            C(255, 60, 70, 0.25f * g_animAlpha), C(255, 100, 100, 0.15f * g_animAlpha),
            C(255, 100, 100, 0.05f * g_animAlpha), C(255, 60, 70, 0.05f * g_animAlpha));

        // round top corners clip
        dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + headerH),
            C(0, 0, 0, 0), panelRnd, ImDrawFlags_RoundCornersTop);

        // header text
        {
            ImVec2 ts = ImGui::CalcTextSize(catName.c_str());
            float sc = headerFs / ImGui::GetFontSize();
            float tx = px + (panelW - ts.x * sc) / 2.f;
            float ty = py + (headerH - ts.y * sc) / 2.f;
            dl->AddText(ImGui::GetFont(), headerFs, ImVec2(tx, ty),
                C(255, 255, 255, g_animAlpha), catName.c_str());
        }

        // separator line under header
        dl->AddLine(ImVec2(px + panelW * 0.08f, py + headerH),
                    ImVec2(px + panelW * 0.92f, py + headerH),
                    C(255, 255, 255, 0.08f * g_animAlpha), 1.f);

        // module rows
        float rowY = py + headerH + padY;
        for (int mi = 0; mi < modCount; mi++) {
            auto* m = catMods[mi];
            bool en = m->enabled ? *m->enabled : false;

            float rx = px + sH * 0.003f;
            float rw = panelW - sH * 0.006f;
            float ry = rowY;
            float rowRnd = rnd(6, sH);

            // row bg
            if (en) {
                // enabled: warm salmon/pink gradient
                dl->AddRectFilledMultiColor(
                    ImVec2(rx, ry), ImVec2(rx + rw, ry + rowH),
                    C(255, 80, 90, 0.85f * g_animAlpha),
                    C(255, 130, 130, 0.75f * g_animAlpha),
                    C(255, 140, 140, 0.65f * g_animAlpha),
                    C(255, 90, 100, 0.75f * g_animAlpha));
                // round the gradient manually with a clipped filled rect
                dl->AddRectFilled(ImVec2(rx, ry), ImVec2(rx + rw, ry + rowH),
                    C(0, 0, 0, 0), rowRnd);
            } else {
                // disabled: dark
                dl->AddRectFilled(ImVec2(rx, ry), ImVec2(rx + rw, ry + rowH),
                    C(17, 17, 20, 0.85f * g_animAlpha), rowRnd);
            }

            // subtle shadow under each row
            dl->AddRectFilled(ImVec2(rx, ry + rowH), ImVec2(rx + rw, ry + rowH + 1.5f),
                C(0, 0, 0, 0.15f * g_animAlpha), 0.f);

            // module name text centered
            {
                ImVec2 ts = ImGui::CalcTextSize(m->name.c_str());
                float sc = fontSize / ImGui::GetFontSize();
                float tx = rx + (rw - ts.x * sc) / 2.f;
                float ty = ry + (rowH - ts.y * sc) / 2.f;
                dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx, ty),
                    C(255, 255, 255, g_animAlpha), m->name.c_str());
            }

            // click to toggle
            bool inRow = (mp.x >= rx && mp.x <= rx + rw && mp.y >= ry && mp.y <= ry + rowH);
            if (inRow && mouseClicked && !cs.dragging && m->toggle) {
                m->toggle();
                bool ne = m->enabled ? *m->enabled : false;
                notifications::notify(m->name + (ne ? " enabled" : " disabled"));
            }
            if (inRow) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            rowY += rowH + rowGap;
        }

        if (inHeader) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        catIdx++;
    }
}

} // namespace edu::gui
