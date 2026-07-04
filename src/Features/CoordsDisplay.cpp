#include "CoordsDisplay.h"
#include "../Client/ClientStore.h"
#include "../Minecraft/Actor.h"
#include "../Minecraft/Components.h"

#include <imgui.h>
#include <cstdio>

namespace edu::features {

bool g_coordsEnabled = false;
int g_coordsPosition = 0;

void toggleCoords() { g_coordsEnabled = !g_coordsEnabled; }

void renderCoords() {
    if (!g_coordsEnabled) return;
    if (!edu::isOnHudScreen()) return;

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
    float sW = io.DisplaySize.x, sH = io.DisplaySize.y;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    ImVec2 ts = ImGui::CalcTextSize(buf);
    float sc = fontSize / ImGui::GetFontSize();
    float tw = ts.x * sc;
    float pad = 10.f;

    float tx, ty;
    switch (g_coordsPosition) {
    default:
    case 0: tx = pad;          ty = sH - fontSize - pad; break;
    case 1: tx = sW - tw - pad; ty = sH - fontSize - pad; break;
    case 2: tx = pad;          ty = pad;                  break;
    case 3: tx = sW - tw - pad; ty = pad;                  break;
    }

    dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx + 1, ty + 1),
        IM_COL32(0, 0, 0, 180), buf);
    dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tx, ty),
        IM_COL32(255, 255, 255, 230), buf);
}

} // namespace edu::features
