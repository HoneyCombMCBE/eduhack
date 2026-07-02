#include "ClickGUI.h"
#include "Notifications.h"
#include "../Client/ModuleManager.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <vector>
#include <string>

// 1:1 port of flarial ClickGUI. Using their exact red color palette.

namespace edu::gui {

static bool  g_open = false;
static float g_baseHeightActual = 0.00001f;
static float g_baseHeightReal   = 0.0001f;
static float g_modcardOffset    = 0.f;
static float g_modcardOpacity   = 1.f;

void toggle() { g_open = !g_open; }
bool isOpen() { return g_open; }

static void fl(float& a, float b, float t) {
    a = std::ceilf((a + (b - a) * t) * 1000.f) / 1000.f;
}
static ImVec4 lerpColor(ImVec4 a, ImVec4 b, float t) {
    return ImVec4(a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t, a.w+(b.w-a.w)*t);
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
static ImU32 C4(ImVec4 v) {
    return IM_COL32((int)(v.x*255),(int)(v.y*255),(int)(v.z*255),(int)(v.w*255));
}

// Flarial default colors (exact hex from ClickGUI.hpp defaultConfig)
// primary1:     ff233a   secondary3: 120e0f   modcard1: 201a1b
// primary2:     ffffff   secondary6: ff2438   modcard3: 3f2a2d
// primary3:     9a6b72   secondary8: 302728   modCardEnabled:  188830
// secondary2:   201a1b   modNameText: 8b767a  modCardDisabled: 7d1820
// flariallogo:  FE4443   globalText:  ffffff

static void ShadowRect(ImDrawList* dl, float x, float y, float w, float h,
                        float rounding, float shadowSize) {
    int n = (int)shadowSize;
    if (n < 1) return;
    for (int i = n; i >= 1; i--) {
        float e = (float)i;
        float a = 0.7f * (1.f - e / (n + 1.f));
        a *= a;
        dl->AddRectFilled(ImVec2(x-e,y-e), ImVec2(x+w+e,y+h+e), C(0,0,0,a), rounding);
    }
}

static void drawText(ImDrawList* dl, float x, float y, float maxW, float maxH,
                      const char* text, float fontSize, ImU32 color, bool center = false) {
    float fs = fontSize * 0.18f;
    if (fs < 6.f) fs = 6.f;
    ImVec2 sz = ImGui::CalcTextSize(text);
    float sc = fs / ImGui::GetFontSize();
    float tx = x, ty = y + (maxH - sz.y * sc) / 2.f;
    if (center) tx = x + (maxW - sz.x * sc) / 2.f;
    dl->AddText(ImGui::GetFont(), fs, ImVec2(tx, ty), color, text);
}

struct CardAnim {
    float w, h, sw, sh;
    ImVec4 btnCol;
    float darken;
    bool init = false;
};
static std::vector<CardAnim> g_cards;

static float g_tw1 = 0.000001f, g_tw2 = 0.000001f;
static ImVec4 g_tabCol1, g_tabCol2;
static bool g_tabColInit = false;
static std::string g_tab = "modules";

void applyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = s.ChildRounding = s.FrameRounding = 0;
    s.WindowPadding = s.FramePadding = s.ItemSpacing = ImVec2(0,0);
    s.WindowBorderSize = s.ChildBorderSize = 0;
    s.ScrollbarSize = 5; s.ScrollbarRounding = 3;
    auto* cl = s.Colors;
    cl[ImGuiCol_WindowBg] = cl[ImGuiCol_ChildBg] = ImVec4(0,0,0,0);
    cl[ImGuiCol_ScrollbarBg]   = ImVec4(0,0,0,0.08f);
    cl[ImGuiCol_ScrollbarGrab] = ImVec4(1.f,0.14f,0.22f,0.35f);
}

// Flarial exact colors
static const ImVec4 kSecondary6(1.f, 0.141f, 0.220f, 1.f);    // #ff2438
static const ImVec4 kSecondary8(0.188f, 0.153f, 0.157f, 1.f);  // #302728

static void ModCard(ImDrawList* dl, ModuleInfo& mod, float x, float y,
                     const int index, float sH, float ff) {
    if (index >= (int)g_cards.size()) g_cards.resize(index + 1);
    auto& ca = g_cards[index];
    bool enabled = mod.enabled ? *mod.enabled : false;

    if (!ca.init) {
        ca.w = 0.19f * sH; // flarial initial width (lerps to 0.178)
        ca.h = 0.141f * sH;
        ca.sw = 0.01f; ca.sh = 0.01f;
        // modCardEnabled #188830, modCardDisabled #7d1820
        ca.btnCol = enabled ? ImVec4(0.094f,0.533f,0.188f,1) : ImVec4(0.490f,0.094f,0.125f,1);
        ca.darken = 0.f;
        ca.init = true;
    }

    float roundVal = rnd(34, sH);
    float W = ca.w, H = ca.h;

    ImVec2 mp = ImGui::GetIO().MousePos;
    bool hovered = (mp.x >= x && mp.x <= x + W && mp.y >= y && mp.y <= y + H);

    if (hovered) {
        fl(ca.w, 0.186f*sH, 0.15f*ff);
        fl(ca.h, 0.149f*sH, 0.15f*ff);
        fl(ca.sw, W, 0.25f*ff);
        fl(ca.sh, H, 0.25f*ff);
    } else {
        fl(ca.w, 0.178f*sH, 0.15f*ff);
        fl(ca.h, 0.141f*sH, 0.15f*ff);
        fl(ca.sw, 0.01f, 0.01f*ff);
        fl(ca.sh, 0.01f, 0.01f*ff);
    }

    float diffX = (ca.w - 0.178f*sH) / 2.f;
    float diffY = (ca.h - 0.141f*sH) / 2.f;
    x -= diffX; y -= diffY;
    W = ca.w; H = ca.h;

    // shadow threshold: 0.255 * pushed context height (baseHeightActual*sH)
    if (ca.sw > 0.255f * g_baseHeightActual * sH) {
        ShadowRect(dl, x, y, W, H, roundVal, diffX * 5);
    }

    // modcard1: #201a1b
    dl->AddRectFilled(ImVec2(x,y), ImVec2(x+W,y+H), C(32,26,27), roundVal);

    // icon bg: modcard3 #3f2a2d at (40%W, 11%H), size 0.28*H
    float iconBgSz = 0.28f * H;
    dl->AddRectFilled(ImVec2(x+0.40f*W, y+0.11f*H),
        ImVec2(x+0.40f*W+iconBgSz, y+0.11f*H+iconBgSz), C(63,42,45), 7.5f);

    // icon letter at (43%W, 15%H), size 0.20*H
    float iconImgSz = 0.20f * H;
    float iix = x+0.43f*W, iiy = y+0.15f*H;
    {
        char ch[2] = { mod.name[0], 0 };
        float fs = iconImgSz * 0.7f;
        ImVec2 ts = ImGui::CalcTextSize(ch);
        float sc = fs / ImGui::GetFontSize();
        dl->AddText(ImGui::GetFont(), fs,
            ImVec2(iix+(iconImgSz-ts.x*sc)/2.f, iiy+(iconImgSz-ts.y*sc)/2.f),
            C(26,19,19,0.85f), ch); // modicon #1A1313
    }

    // mod name: modNameText #8b767a, centered, font=0.854*H, textWidth=H
    drawText(dl, x+0.119f*W, y+0.415f*H, H, H*0.2f,
             mod.name.c_str(), 0.854f*H, C(139,118,122), true);

    // button dims
    float buttonWidth = 0.68f*W, buttonHeight = 0.259f*H;
    float buttonx = x + W*0.96f, buttony = y + H*0.907f;
    float paddingwidth = 0.26f * H;

    // settings gear area: modcard3 #3f2a2d
    float settingx = x + 0.0485f*W;
    dl->AddRectFilled(ImVec2(settingx, buttony-buttonHeight),
        ImVec2(settingx + paddingwidth + 0.26f*H, buttony), C(63,42,45), rnd(20,sH));

    // gear icon
    float gearX = x + 0.0885f*W;
    float gearY2 = (buttony-buttonHeight) + 0.162f*paddingwidth;
    float gW = 0.17f*H, giW = 0.15f*H;
    float gcx = gearX+(gW-giW)/2.f+giW/2.f, gcy = gearY2+(gW-giW)/2.f+giW/2.f;
    float gR = giW*0.38f;
    dl->AddCircle(ImVec2(gcx,gcy), gR, C(255,255,255,0.75f), 12, 1.3f);
    for (int t = 0; t < 8; t++) {
        float a = t*6.28318f/8.f;
        dl->AddLine(ImVec2(gcx+cosf(a)*gR*0.8f,gcy+sinf(a)*gR*0.8f),
                    ImVec2(gcx+cosf(a)*gR*1.3f,gcy+sinf(a)*gR*1.3f), C(255,255,255,0.55f), 1.8f);
    }
    dl->AddCircleFilled(ImVec2(gcx,gcy), gR*0.3f, C(63,42,45));

    // button color: enabled #188830, disabled #7d1820
    ImVec4 enCol(0.094f,0.533f,0.188f,1), disCol(0.490f,0.094f,0.125f,1);
    ImVec4 tgt = enabled ? enCol : disCol;
    if (hovered) fl(ca.darken, 0.04f, 0.15f*ff);
    else fl(ca.darken, 0.f, 0.15f*ff);
    ca.btnCol = lerpColor(ca.btnCol, tgt, 0.15f*ff);
    ImVec4 bd(ca.btnCol.x-ca.darken, ca.btnCol.y-ca.darken, ca.btnCol.z-ca.darken, ca.btnCol.w);

    // color bar behind button (rounding 0)
    float barX = (buttonx-buttonWidth) - 0.105f*paddingwidth;
    float barW = buttonWidth - 1.5f*paddingwidth;
    dl->AddRectFilled(ImVec2(barX,buttony-buttonHeight), ImVec2(barX+barW,buttony), C4(bd), 0.f);

    // main button
    dl->AddRectFilled(ImVec2(buttonx-buttonWidth,buttony-buttonHeight),
        ImVec2(buttonx,buttony), C4(bd), rnd(22,sH));

    // button text: globalText #ffffff
    const char* btnText = enabled ? "Enabled" : "Disabled";
    drawText(dl, (buttonx-buttonWidth)-0.15f*paddingwidth, buttony-buttonHeight,
             buttonWidth, buttonHeight, btnText, buttonWidth*1.08f, C(255,255,255), true);

    if (hovered && ImGui::IsMouseClicked(0) && mod.toggle) {
        mod.toggle();
        bool ne = mod.enabled ? *mod.enabled : false;
        notifications::notify(mod.name + (ne ? " enabled" : " disabled"));
    }
    if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
}

void render() {
    auto& io = ImGui::GetIO();
    float dt = io.DeltaTime > 0 ? io.DeltaTime : 0.016f;
    float ff = ff_dt(dt);
    float sW = io.DisplaySize.x, sH = io.DisplaySize.y;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    if (g_open) {
        fl(g_baseHeightActual, 0.64f, 0.18f*ff);
        fl(g_baseHeightReal,   0.64f, 0.28f*ff);
    } else {
        fl(g_baseHeightReal,   0.0001f, 0.22f*ff);
        fl(g_baseHeightActual, 0.00001f, 0.30f*ff);
    }

    if (g_baseHeightActual < 0.1f) return;

    if (!g_tabColInit) { g_tabCol1 = kSecondary6; g_tabCol2 = kSecondary8; g_tabColInit = true; }

    // base rect: secondary3 #120e0f
    float baseW = 0.81f*sH, baseH = g_baseHeightReal*sH;
    float baseRnd = rnd(43,sH);
    float baseX = (sW-baseW)/2.f, baseY = (sH-baseH)/2.f;

    dl->AddRectFilled(ImVec2(baseX,baseY), ImVec2(baseX+baseW,baseY+baseH),
        C(18,14,15), baseRnd);

    // nav bar: secondary2 #201a1b (relative to baseHeightActual like ref PushSize)
    float navW = g_baseHeightActual*sH*1.235f;
    float navH = g_baseHeightActual*sH*0.124f;
    float navX = baseX + baseW*0.013f, navY = baseY + baseH*0.019f;

    dl->AddRectFilled(ImVec2(navX,navY), ImVec2(navX+navW,navY+navH), C(32,26,27), rnd(28,sH));

    // logo: flariallogo #FE4443, x = navx + 0.55*logo
    float logoWR = navH*0.5f;
    float logoX = navX - 0.05f*logoWR + logoWR*0.6f;
    float logoY = navY + navH/2.f - logoWR/2.f;
    dl->AddText(ImGui::GetFont(), logoWR*0.85f,
        ImVec2(logoX, logoY+logoWR*0.1f), C(254,68,67), "Aura");

    // tabs
    float tabBaseW = navH*0.448f;
    float RadioBtnW = navW*0.134f, RadioBtnH = tabBaseW;
    float tabRnd = rnd(17.5f,sH);

    fl(g_tw1, g_tab=="modules"  ? RadioBtnW : tabBaseW, 0.15f*ff);
    fl(g_tw2, g_tab=="settings" ? RadioBtnW : tabBaseW, 0.15f*ff);

    g_tabCol1 = lerpColor(g_tabCol1, g_tab=="modules"  ? kSecondary6 : kSecondary8, 0.15f*ff);
    g_tabCol2 = lerpColor(g_tabCol2, g_tab=="settings" ? kSecondary6 : kSecondary8, 0.15f*ff);

    float logoWidth = navH*1.21f;
    float radioX = navX + 0.85f*logoWidth;
    float radioY = navY + navH/2.f - RadioBtnH/2.f;

    // tab 1
    ShadowRect(dl, radioX, radioY+0.015f*logoWidth, g_tw1, RadioBtnH+0.015f*logoWidth, tabRnd, 3);
    dl->AddRectFilled(ImVec2(radioX,radioY), ImVec2(radioX+g_tw1,radioY+RadioBtnH), C4(g_tabCol1), tabRnd);

    float logoSmall = tabBaseW*0.625f;
    if (g_tw1 > tabBaseW*1.3f) {
        float fs = logoSmall*0.5f;
        ImVec2 ts = ImGui::CalcTextSize("Modules");
        float sc = fs/ImGui::GetFontSize();
        dl->AddText(ImGui::GetFont(), fs,
            ImVec2(radioX+(g_tw1-ts.x*sc)/2.f, radioY+(RadioBtnH-ts.y*sc)/2.f), C(255,255,255), "Modules");
    } else {
        float isz = logoSmall*0.55f;
        float ix = radioX+(g_tw1-isz)/2.f, iy = radioY+(RadioBtnH-isz)/2.f;
        dl->AddRectFilled(ImVec2(ix,iy), ImVec2(ix+isz,iy+isz*0.7f), C(255,255,255,0.6f), 2.f);
    }
    { ImVec2 m=io.MousePos; if(m.x>=radioX&&m.x<=radioX+g_tw1&&m.y>=radioY&&m.y<=radioY+RadioBtnH) {
        if(ImGui::IsMouseClicked(0)) g_tab="modules"; ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); } }

    logoWidth = tabBaseW*0.625f; // reassign after tab1 (flarial line 648)
    float tab2X = radioX + g_tw1 + 0.9f*logoWidth;

    // tab 2
    ShadowRect(dl, tab2X, radioY+0.115f*logoWidth, g_tw2, RadioBtnH+0.015f*logoWidth, tabRnd, 3);
    dl->AddRectFilled(ImVec2(tab2X,radioY), ImVec2(tab2X+g_tw2,radioY+RadioBtnH), C4(g_tabCol2), tabRnd);

    if (g_tw2 > tabBaseW*1.3f) {
        float fs = logoSmall*0.5f;
        ImVec2 ts = ImGui::CalcTextSize("Settings");
        float sc = fs/ImGui::GetFontSize();
        dl->AddText(ImGui::GetFont(), fs,
            ImVec2(tab2X+(g_tw2-ts.x*sc)/2.f, radioY+(RadioBtnH-ts.y*sc)/2.f), C(255,255,255), "Settings");
    } else {
        float isz = logoSmall*0.55f;
        float ix = tab2X+(g_tw2-isz)/2.f, iy = radioY+(RadioBtnH-isz)/2.f;
        dl->AddCircle(ImVec2(ix+isz/2.f,iy+isz/2.f), isz*0.35f, C(255,255,255,0.6f), 8, 1.3f);
    }
    { ImVec2 m=io.MousePos; if(m.x>=tab2X&&m.x<=tab2X+g_tw2&&m.y>=radioY&&m.y<=radioY+RadioBtnH) {
        if(ImGui::IsMouseClicked(0)) g_tab="settings"; ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); } }

    // search bar: secondary4 #1c1616, rounding 19
    float sbH = navH*0.42f, sbW = sbH;
    float sbX = navX+navW-sbW-navW*0.022f, sbY = navY+navH/2.f-sbH/2.f;
    dl->AddRectFilled(ImVec2(sbX,sbY), ImVec2(sbX+sbW,sbY+sbH), C(28,22,22), rnd(19,sH));

    // content
    float contentY = navY+navH+baseH*0.018f;
    float contentH = (baseY+baseH-baseH*0.012f) - contentY;

    dl->PushClipRect(ImVec2(baseX+2,contentY), ImVec2(baseX+baseW-2,baseY+baseH-2), true);

    if (g_tab == "modules") {
        // card opacity & slide (flarial modcardOpacity + modcardOffset)
        fl(g_modcardOpacity, 1.f, 0.09f*ff);
        fl(g_modcardOffset, 0.f, 0.245f*ff);

        float modWidth = 0.19f*sH;
        float modHeight = 0.1369f*sH;
        float xSpacing = 1.02f*modWidth;
        float ySpacing = 0.8f*modWidth;

        // CenterConstraint(modWidth, modHeight, "both", -0.58, -0.52)
        // = baseX + (baseW - modWidth)/2 + (-0.58)*(baseW-modWidth)/2
        // = baseX + (baseW - modWidth)/2 * (1 - 0.58)
        float startX = baseX + (baseW - modWidth)/2.f * (1.f - 0.58f);

        ImGui::SetNextWindowPos(ImVec2(baseX+baseW*0.02f, contentY));
        ImGui::SetNextWindowSize(ImVec2(baseW*0.96f, contentH));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(g_baseHeightActual/0.64f, 0.f, 1.f) * g_modcardOpacity);
        if (ImGui::Begin("##cards", nullptr,
                ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
                ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBackground|
                ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoBringToFrontOnFocus)) {

            auto& mods = edu::getModules();
            int c2 = 0; float cy = 0;
            for (int j = 0; j < (int)mods.size(); j++) {
                float cx = (float)c2 * xSpacing + g_modcardOffset;
                float sx = startX + cx;
                float sy = ImGui::GetCursorScreenPos().y + cy;
                ModCard(dl, mods[j], sx, sy, j, sH, ff);
                c2++;
                if (c2 >= 3) { c2 = 0; cy += ySpacing; }
            }
            ImGui::Dummy(ImVec2(0, cy + modHeight));
        }
        ImGui::End();
        ImGui::PopStyleVar();

    } else {
        // when NOT on modules tab, fade out cards and slide left
        fl(g_modcardOpacity, 0.f, 0.09f*ff);
        fl(g_modcardOffset, -2.f*sH, 0.08f*ff);

        if (g_tab == "settings") {
            float px = baseX+baseW*0.04f, py = contentY+baseH*0.02f;
            float fs = sH*0.016f;
            dl->AddText(ImGui::GetFont(), fs*1.2f, ImVec2(px,py), C(255,35,58), "Aura Client Settings");
            py += fs*2.5f;
            dl->AddText(ImGui::GetFont(), fs, ImVec2(px,py), C(139,118,122), "No configurable settings yet.");
        }
    }

    // top/bottom scroll edge shadows AFTER content so they overlay cards
    float shadowW2 = baseW * 0.74f;
    float shadowX2 = baseX + baseW * 0.15f;
    float shadowH2 = sH*0.03f;
    dl->AddRectFilledMultiColor(ImVec2(shadowX2,contentY), ImVec2(shadowX2+shadowW2,contentY+shadowH2),
        C(18,14,15,0.9f), C(18,14,15,0.9f), C(18,14,15,0.f), C(18,14,15,0.f));
    dl->AddRectFilledMultiColor(ImVec2(shadowX2,baseY+baseH-2-shadowH2), ImVec2(shadowX2+shadowW2,baseY+baseH-2),
        C(18,14,15,0.f), C(18,14,15,0.f), C(18,14,15,0.9f), C(18,14,15,0.9f));

    dl->PopClipRect();
}

} // namespace edu::gui
