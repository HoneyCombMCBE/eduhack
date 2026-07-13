#include "DevOverlay.h"
#include "../Client/ClientStore.h"
#include "../Client/ClientInstance.h"
#include <imgui.h>

namespace edu::gui::devOverlay {

static bool g_open = false;
static char g_ip[128] = "127.0.0.1";
static int g_port = 19132;

bool isOpen() {
    return g_open;
}

void toggle() {
    auto* ci = edu::getClientInstance();
    if (!ci) return;

    g_open = !g_open;
    if (g_open) {
        ci->releaseMouse();
    } else {
        ci->grabMouse();
    }
}

void render() {
    if (!g_open) return;

    auto* ci = edu::getClientInstance();
    if (!ci) return;

    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Developer Overlay", &g_open)) {
        ImGui::Text("Screen: %s", ci->getScreenName().c_str());

        void* player = ci->getLocalPlayer();
        if (player) {
            ImGui::Text("Player status: Active");
        } else {
            ImGui::Text("Player status: Inactive");
        }

        ImGui::Separator();
        ImGui::Text("Server Connector");
        
        ImGui::InputText("IP Address", g_ip, sizeof(g_ip));
        ImGui::InputInt("Port", &g_port);

        if (ImGui::Button("Connect to Third Party Server")) {
            ci->connectToThirdPartyServer(g_ip, g_port);
        }
    }
    ImGui::End();

    // If closed via the ImGui close button
    if (!g_open) {
        ci->grabMouse();
    }
}

} // namespace edu::gui::devOverlay
