#include "ClickGUI.h"
#include "Notifications.h"
#include "../Client/ModuleManager.h"
#include "../Client/Chat.h"
#include "../Client/ClientStore.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <windows.h>


namespace edu::gui {

static bool g_open = false;
static int g_selCat = 0;
static int g_selMod = 0;

void toggle() {
    auto* ci = edu::getClientInstance();
    if (!ci) return;

    if (!g_open) {
        std::string screen = ci->getScreenName();
        if (screen != "hud_screen" && screen != "pause_screen" &&
            screen != "f3_screen" && screen != "zoom_screen")
            return;
        g_open = true;
    } else {
        g_open = false;
    }
}
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
                        float rounding, int layers, float alpha) {
    if (layers < 1) return;
    for (int i = layers; i >= 1; i--) {
        float e = (float)i * 1.5f;
        float a2 = 0.5f * (1.f - (float)i / (layers + 1.f));
        a2 *= a2;
        dl->AddRectFilled(ImVec2(x-e,y-e), ImVec2(x+w+e,y+h+e), C(0,0,0,a2 * alpha), rounding);
    }
}

static float g_animAlpha = 0.f;

static bool keyPressed(int vk) {
    static bool prev[256] = {};
    bool now = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool pressed = now && !prev[vk];
    prev[vk] = now;
    return pressed;
}

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
    if (g_animAlpha < 0.005f) return;

    auto cats = edu::getCategories();
    auto& mods = edu::getModules();
    int catCount = (int)cats.size();
    if (catCount == 0) return;

    if (g_selCat >= catCount) g_selCat = catCount - 1;

    auto getModsForCat = [&](int ci) -> std::vector<ModuleInfo*> {
        std::vector<ModuleInfo*> out;
        for (auto& m : mods)
            if (m.category == cats[ci]) out.push_back(&m);
        return out;
    };

    if (g_open) {
        if (keyPressed(VK_LEFT))  { g_selCat = (g_selCat - 1 + catCount) % catCount; g_selMod = 0; }
        if (keyPressed(VK_RIGHT)) { g_selCat = (g_selCat + 1) % catCount; g_selMod = 0; }

        auto curMods = getModsForCat(g_selCat);
        int modCount = (int)curMods.size();

        if (keyPressed(VK_UP))   g_selMod = (g_selMod - 1 + modCount) % modCount;
        if (keyPressed(VK_DOWN)) g_selMod = (g_selMod + 1) % modCount;

        if (keyPressed(VK_RETURN) && g_selMod < modCount) {
            auto* m = curMods[g_selMod];
            if (m->toggle) {
                m->toggle();
                bool ne = m->enabled ? *m->enabled : false;
                notifications::notify(m->name + (ne ? " enabled" : " disabled"));
            }
        }
    }

    float panelW   = sH * 0.28f;
    float headerH  = sH * 0.05f;
    float rowH     = sH * 0.042f;
    float rowGap   = sH * 0.002f;
    float padY     = sH * 0.008f;
    float panelRnd = rnd(18, sH);
    float fontSize = sH * 0.020f;
    float headerFs = sH * 0.024f;

    float totalW = (float)catCount * panelW + (float)(catCount - 1) * sH * 0.02f;
    float startX = (sW - totalW) / 2.f;
    float startY = sH * 0.12f;

    for (int ci = 0; ci < catCount; ci++) {
        auto& catName = cats[ci];
        bool isSel = (ci == g_selCat);

        auto catMods = getModsForCat(ci);
        int modCount = (int)catMods.size();
        float panelH = headerH + padY + modCount * (rowH + rowGap) + padY;

        float px = startX + ci * (panelW + sH * 0.02f);
        float py = startY;

        ShadowRect(dl, px, py, panelW, panelH, panelRnd, 8, g_animAlpha);

        dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + panelH),
            C(22, 22, 26, 0.95f * g_animAlpha), panelRnd);

        if (isSel) {
            dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + headerH + panelRnd),
                C(200, 50, 60, 0.9f * g_animAlpha), panelRnd, ImDrawFlags_RoundCornersTop);
        } else {
            dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + headerH + panelRnd),
                C(45, 30, 32, 0.8f * g_animAlpha), panelRnd, ImDrawFlags_RoundCornersTop);
            dl->AddRectFilledMultiColor(
                ImVec2(px + 1, py + 1), ImVec2(px + panelW - 1, py + headerH),
                C(200, 50, 60, 0.35f * g_animAlpha), C(220, 80, 80, 0.25f * g_animAlpha),
                C(200, 70, 70, 0.05f * g_animAlpha), C(180, 45, 55, 0.10f * g_animAlpha));
        }

        {
            ImVec2 ts = ImGui::CalcTextSize(catName.c_str());
            float sc = headerFs / ImGui::GetFontSize();
            float tx = px + (panelW - ts.x * sc) / 2.f;
            float ty = py + (headerH - ts.y * sc) / 2.f;
            dl->AddText(ImGui::GetFont(), headerFs, ImVec2(tx, ty),
                C(255, 255, 255, g_animAlpha), catName.c_str());
        }

        dl->AddLine(ImVec2(px + panelW * 0.06f, py + headerH),
                    ImVec2(px + panelW * 0.94f, py + headerH),
                    C(255, 255, 255, 0.06f * g_animAlpha), 1.f);

        float rowY = py + headerH + padY;
        for (int mi = 0; mi < modCount; mi++) {
            auto* m = catMods[mi];
            bool en = m->enabled ? *m->enabled : false;
            bool highlighted = isSel && mi == g_selMod;

            float rowPad = sH * 0.004f;
            float rx = px + rowPad;
            float rw = panelW - rowPad * 2.f;
            float ry = rowY;
            float rowRnd2 = rnd(8, sH);

            if (en) {
                dl->AddRectFilled(ImVec2(rx, ry), ImVec2(rx + rw, ry + rowH),
                    C(255, 75, 85, 0.80f * g_animAlpha), rowRnd2);
                dl->AddRectFilledMultiColor(
                    ImVec2(rx, ry), ImVec2(rx + rw, ry + rowH),
                    C(255, 90, 100, 0.3f * g_animAlpha),
                    C(255, 150, 140, 0.3f * g_animAlpha),
                    C(255, 140, 130, 0.15f * g_animAlpha),
                    C(255, 80, 90, 0.15f * g_animAlpha));
            } else {
                dl->AddRectFilled(ImVec2(rx, ry), ImVec2(rx + rw, ry + rowH),
                    C(15, 15, 18, 0.90f * g_animAlpha), rowRnd2);
            }

            if (highlighted) {
                dl->AddRect(ImVec2(rx, ry), ImVec2(rx + rw, ry + rowH),
                    C(255, 255, 255, 0.9f * g_animAlpha), rowRnd2, 0, 2.f);
            }

            dl->AddRectFilled(ImVec2(rx + 2, ry + rowH), ImVec2(rx + rw - 2, ry + rowH + 2.f),
                C(0, 0, 0, 0.12f * g_animAlpha), 1.f);

            {
                ImVec2 ts = ImGui::CalcTextSize(m->name.c_str());
                float sc = fontSize / ImGui::GetFontSize();
                float tx2 = rx + (rw - ts.x * sc) / 2.f;
                float ty2 = ry + (rowH - ts.y * sc) / 2.f;
                dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx2, ty2),
                    C(255, 255, 255, g_animAlpha), m->name.c_str());
            }

            rowY += rowH + rowGap;
        }
    }
}

} // namespace edu::gui
