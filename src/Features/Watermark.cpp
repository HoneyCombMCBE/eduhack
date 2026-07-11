#include "Watermark.h"
#include "../Client/ClientStore.h"
#include "../Client/Theme.h"
#include <imgui.h>
#include <string>

namespace edu::features {

bool g_watermarkEnabled = true;

void toggleWatermark() { g_watermarkEnabled = !g_watermarkEnabled; }

void renderWatermark() {
    if (!g_watermarkEnabled) return;
    if (!edu::isOnHudScreen()) return;

    auto& io = ImGui::GetIO();
    float sH = io.DisplaySize.y;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    float fontSize = sH * 0.045f;
    ImVec2 pos = ImVec2(12.f, 12.f);

    std::string text = "heheboi";

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        std::string charStr(1, c);
        ImVec2 charSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, charStr.c_str());

        // Get themed color based on character index
        ImColor color = edu::getThemeColor(static_cast<float>(i * 100));

        // Draw shadow (darker character color offset by 3.25f, matching Solstice)
        ImColor shadowColor = ImColor(
            color.Value.x * 0.25f, 
            color.Value.y * 0.25f, 
            color.Value.z * 0.25f, 
            0.925f
        );
        dl->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 3.25f, pos.y + 3.25f), shadowColor, charStr.c_str());

        // Draw character
        dl->AddText(ImGui::GetFont(), fontSize, pos, color, charStr.c_str());

        pos.x += charSize.x;
    }
}

} // namespace edu::features
