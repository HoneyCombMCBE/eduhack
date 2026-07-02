#include "ClickGUI.h"
#include "Notifications.h"
#include "../Client/ModuleManager.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace edu::gui {

static bool  g_open      = false;
static float g_animAlpha = 0.0f;
static float g_animScale = 0.92f;

void toggle() { g_open = !g_open; }
bool isOpen() { return g_open; }

static float lerp(float a, float b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

static float getTime() {
    static auto start = std::chrono::steady_clock::now();
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
}

static ImVec4 tealAccent(float alpha = 1.0f) {
    float t = getTime();
    float r = 0.10f + 0.05f * sinf(t * 0.8f);
    float g = 0.75f + 0.08f * sinf(t * 1.1f + 1.0f);
    float b = 0.78f + 0.06f * sinf(t * 0.9f + 2.0f);
    return ImVec4(r, g, b, alpha);
}

static ImVec4 tealDark(float alpha = 1.0f) {
    return ImVec4(0.06f, 0.20f, 0.22f, alpha);
}

void applyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 14.0f;
    s.ChildRounding     = 12.0f;
    s.FrameRounding     = 10.0f;
    s.GrabRounding      = 8.0f;
    s.PopupRounding     = 10.0f;
    s.WindowPadding     = ImVec2(18, 18);
    s.FramePadding      = ImVec2(12, 7);
    s.ItemSpacing       = ImVec2(10, 8);
    s.WindowBorderSize  = 0.0f;
    s.ChildBorderSize   = 0.0f;
    s.ScrollbarSize     = 8.0f;
    s.ScrollbarRounding = 10.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]        = ImVec4(0.05f, 0.07f, 0.09f, 0.97f);
    c[ImGuiCol_ChildBg]         = ImVec4(0.07f, 0.09f, 0.12f, 1.0f);
    c[ImGuiCol_Border]          = ImVec4(0.12f, 0.25f, 0.28f, 0.4f);
    c[ImGuiCol_Text]            = ImVec4(0.92f, 0.96f, 0.97f, 1.0f);
    c[ImGuiCol_TextDisabled]    = ImVec4(0.40f, 0.50f, 0.52f, 1.0f);
    c[ImGuiCol_Button]          = ImVec4(0.08f, 0.14f, 0.16f, 1.0f);
    c[ImGuiCol_ButtonHovered]   = ImVec4(0.10f, 0.22f, 0.25f, 1.0f);
    c[ImGuiCol_ButtonActive]    = ImVec4(0.12f, 0.35f, 0.38f, 1.0f);
    c[ImGuiCol_ScrollbarBg]     = ImVec4(0.05f, 0.07f, 0.09f, 0.4f);
    c[ImGuiCol_ScrollbarGrab]   = ImVec4(0.15f, 0.30f, 0.32f, 1.0f);
}

static void drawGradientRect(ImDrawList* dl, ImVec2 p0, ImVec2 p1,
                              ImU32 colTL, ImU32 colTR, ImU32 colBL, ImU32 colBR,
                              float rounding = 0.0f) {
    if (rounding > 0.0f) {
        dl->AddRectFilled(p0, p1, colTL, rounding);
        dl->AddRectFilledMultiColor(p0, p1, colTL, colTR, colBR, colBL);
    } else {
        dl->AddRectFilledMultiColor(p0, p1, colTL, colTR, colBR, colBL);
    }
}

static void renderModuleCard(ModuleInfo& mod, float cardW) {
    bool enabled = mod.enabled ? *mod.enabled : false;
    float t = getTime();

    ImGui::PushID(mod.name.c_str());
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float cardH = 72.0f;

    ImVec2 p0 = cursor;
    ImVec2 p1(cursor.x + cardW, cursor.y + cardH);

    if (enabled) {
        ImVec4 tl = tealDark(0.9f);
        ImVec4 tr = ImVec4(0.04f, 0.12f, 0.18f, 0.9f);
        float pulse = 0.5f + 0.5f * sinf(t * 2.0f);
        tl.x += 0.03f * pulse;
        tl.y += 0.05f * pulse;

        dl->AddRectFilled(p0, p1, ImGui::ColorConvertFloat4ToU32(tl), 12.0f);
        drawGradientRect(dl, p0, p1,
            ImGui::ColorConvertFloat4ToU32(tl),
            ImGui::ColorConvertFloat4ToU32(tr),
            ImGui::ColorConvertFloat4ToU32(tl),
            ImGui::ColorConvertFloat4ToU32(tr));

        ImVec4 ac = tealAccent(0.7f);
        dl->AddRect(p0, p1, ImGui::ColorConvertFloat4ToU32(ac), 12.0f, 0, 1.8f);

        float glowAlpha = 0.04f + 0.02f * sinf(t * 3.0f);
        ImU32 glow = ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.8f, 0.8f, glowAlpha));
        dl->AddRectFilled(p0, p1, glow, 12.0f);
    } else {
        ImU32 bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.07f, 0.09f, 0.11f, 0.95f));
        ImU32 border = ImGui::ColorConvertFloat4ToU32(ImVec4(0.14f, 0.16f, 0.19f, 0.6f));
        dl->AddRectFilled(p0, p1, bg, 12.0f);
        dl->AddRect(p0, p1, border, 12.0f, 0, 1.0f);
    }

    ImVec2 namePos(cursor.x + 18.0f, cursor.y + 15.0f);
    ImU32 nameCol = ImGui::ColorConvertFloat4ToU32(
        enabled ? ImVec4(0.85f, 1.0f, 0.98f, 1.0f) : ImVec4(0.75f, 0.78f, 0.80f, 1.0f));
    dl->AddText(ImGui::GetFont(), 17.0f, namePos, nameCol, mod.name.c_str());

    ImVec2 descPos(cursor.x + 18.0f, cursor.y + 38.0f);
    ImU32 descCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.45f, 0.52f, 0.55f, 1.0f));
    dl->AddText(ImGui::GetFont(), 13.0f, descPos, descCol, mod.description.c_str());

    float dotR = 5.0f;
    ImVec2 dotPos(cursor.x + cardW - 22.0f, cursor.y + cardH / 2.0f);
    if (enabled) {
        float pulse = 0.7f + 0.3f * sinf(t * 3.0f);
        dl->AddCircleFilled(dotPos, dotR + 3.0f,
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.85f, 0.75f, 0.15f * pulse)));
        dl->AddCircleFilled(dotPos, dotR,
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.15f, 0.92f, 0.8f, 1.0f)));
    } else {
        dl->AddCircleFilled(dotPos, dotR,
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.30f, 0.32f, 0.35f, 1.0f)));
    }

    ImGui::SetCursorScreenPos(cursor);
    if (ImGui::InvisibleButton("##card", ImVec2(cardW, cardH))) {
        if (mod.toggle) {
            mod.toggle();
            bool nowEnabled = mod.enabled ? *mod.enabled : false;
            notifications::notify(mod.name + (nowEnabled ? " enabled" : " disabled"));
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::PopID();
}

void render() {
    float dt = ImGui::GetIO().DeltaTime;
    float speed = 10.0f * dt;

    g_animAlpha = lerp(g_animAlpha, g_open ? 1.0f : 0.0f, speed);
    g_animScale = lerp(g_animScale, g_open ? 1.0f : 0.92f, speed);

    if (g_animAlpha < 0.01f) return;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_animAlpha);

    auto& io = ImGui::GetIO();
    float winW = 400.0f;
    float winH = 520.0f;

    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x / 2 - (winW * g_animScale) / 2,
               io.DisplaySize.y / 2 - (winH * g_animScale) / 2),
        ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(winW * g_animScale, winH * g_animScale), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("##edugui", nullptr, flags)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();
        ImVec2 ws = ImGui::GetWindowSize();

        float t = getTime();
        ImVec4 titleTeal = tealAccent(g_animAlpha);
        ImU32 titleCol = ImGui::ColorConvertFloat4ToU32(titleTeal);
        ImU32 subtitleCol = ImGui::ColorConvertFloat4ToU32(
            ImVec4(0.50f, 0.58f, 0.60f, g_animAlpha));

        dl->AddText(ImGui::GetFont(), 24.0f, ImVec2(wp.x + 22, wp.y + 16), titleCol, "Aura");

        float nameW = ImGui::CalcTextSize("Aura").x * (24.0f / ImGui::GetFontSize());
        dl->AddText(ImGui::GetFont(), 24.0f,
                    ImVec2(wp.x + 22 + nameW + 4, wp.y + 16), subtitleCol, "Client");

        ImVec2 lineL(wp.x + 18, wp.y + 52);
        ImVec2 lineR(wp.x + ws.x - 18, wp.y + 52);
        ImU32 lineColL = ImGui::ColorConvertFloat4ToU32(tealAccent(0.5f * g_animAlpha));
        ImU32 lineColR = ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.15f, 0.18f, 0.2f * g_animAlpha));
        dl->AddLine(lineL, lineR, lineColL, 1.5f);

        ImGui::SetCursorPos(ImVec2(14, 62));
        ImGui::BeginChild("##modules", ImVec2(ws.x - 28, ws.y - 76), false);

        float cardW = ImGui::GetContentRegionAvail().x - 4;
        for (auto& mod : edu::getModules()) {
            renderModuleCard(mod, cardW);
            ImGui::Spacing();
        }

        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleVar();
}

} // namespace edu::gui
