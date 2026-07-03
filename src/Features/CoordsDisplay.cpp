#include "CoordsDisplay.h"
#include "../Client/ClientStore.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

#include <imgui.h>
#include <cstdio>

namespace edu::features {

bool g_coordsEnabled = false;

void toggleCoords() { g_coordsEnabled = !g_coordsEnabled; }

void renderCoords() {
    if (!g_coordsEnabled) return;

    auto* ci = edu::getClientInstance();
    if (!ci) return;
    auto* lp = ci->getLocalPlayer();
    if (!lp) return;

    auto* actor = reinterpret_cast<Actor*>(lp);
    auto& ctx = actor->getEntity();
    auto* sv = ctx.tryGetComponent<StateVectorComponent>();
    if (!sv) return;

    char buf[128];
    std::snprintf(buf, sizeof(buf), "%.1f / %.1f / %.1f", sv->pos.x, sv->pos.y, sv->pos.z);

    auto& io = ImGui::GetIO();
    float fontSize = io.DisplaySize.y * 0.022f;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    ImVec2 ts = ImGui::CalcTextSize(buf);
    float sc = fontSize / ImGui::GetFontSize();
    float tx = 10.f;
    float ty = io.DisplaySize.y - fontSize - 10.f;

    dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx + 1, ty + 1),
        IM_COL32(0, 0, 0, 180), buf);
    dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx, ty),
        IM_COL32(255, 255, 255, 230), buf);
}

} // namespace edu::features
