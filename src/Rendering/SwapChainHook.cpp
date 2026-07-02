#include "SwapChainHook.h"
#include "../GUI/ClickGUI.h"
#include "../GUI/Notifications.h"
#include "../Input/KeyInput.h"

#include <d3d11.h>
#include <dxgi.h>
#include <kiero.hpp>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace edu::rendering {

static ID3D11Device*           g_device  = nullptr;
static ID3D11DeviceContext*    g_context = nullptr;
static ID3D11RenderTargetView* g_rtv     = nullptr;
static HWND                    g_hwnd    = nullptr;
static bool                    g_imgui   = false;
static WNDPROC                 g_origWndProc = nullptr;

using PresentFn       = HRESULT(*)(IDXGISwapChain*, UINT, UINT);
using ResizeBuffersFn = HRESULT(*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

static PresentFn       oPresent       = nullptr;
static ResizeBuffersFn oResizeBuffers = nullptr;

static LRESULT CALLBACK hk_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))
        return 1;

    if (gui::isOpen()) {
        if (msg == WM_MOUSEMOVE || msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP ||
            msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP || msg == WM_MOUSEWHEEL)
            return 1;
    }

    return CallWindowProcA(g_origWndProc, hwnd, msg, wp, lp);
}

static bool g_welcomed = false;

static void initImGui(IDXGISwapChain* sc) {
    if (g_imgui) return;

    DXGI_SWAP_CHAIN_DESC desc{};
    sc->GetDesc(&desc);
    g_hwnd = desc.OutputWindow;

    sc->GetDevice(__uuidof(ID3D11Device), (void**)&g_device);
    g_device->GetImmediateContext(&g_context);

    ID3D11Texture2D* backBuf = nullptr;
    sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuf);
    g_device->CreateRenderTargetView(backBuf, nullptr, &g_rtv);
    backBuf->Release();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    gui::applyStyle();

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);

    g_origWndProc = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)hk_WndProc);

    g_imgui = true;
}

static HRESULT hk_Present(IDXGISwapChain* sc, UINT sync, UINT flags) {
    initImGui(sc);

    if (input::isJustPressed(VK_RSHIFT))
        gui::toggle();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (!g_welcomed) {
        SetWindowTextA(g_hwnd, "Aura Client");
        gui::notifications::notify("Aura Client loaded");
        g_welcomed = true;
    }

    gui::render();
    gui::notifications::render();

    ImGui::Render();
    g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(sc, sync, flags);
}

static HRESULT hk_ResizeBuffers(IDXGISwapChain* sc, UINT count, UINT w, UINT h,
                                 DXGI_FORMAT fmt, UINT fl) {
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }

    HRESULT hr = oResizeBuffers(sc, count, w, h, fmt, fl);

    ID3D11Texture2D* backBuf = nullptr;
    sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuf);
    if (backBuf) {
        g_device->CreateRenderTargetView(backBuf, nullptr, &g_rtv);
        backBuf->Release();
    }
    return hr;
}

bool installSwapChainHook() {
    if (kiero::init(kiero::RenderType::Auto) != kiero::Status::Success)
        return false;

    kiero::bind<&IDXGISwapChain::Present>(&oPresent, &hk_Present);
    kiero::bind<&IDXGISwapChain::ResizeBuffers>(&oResizeBuffers, &hk_ResizeBuffers);
    return true;
}

void removeSwapChainHook() {
    if (g_origWndProc && g_hwnd)
        SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)g_origWndProc);

    if (g_imgui) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    if (g_rtv) g_rtv->Release();
    if (g_context) g_context->Release();
    if (g_device) g_device->Release();
}

} // namespace edu::rendering
