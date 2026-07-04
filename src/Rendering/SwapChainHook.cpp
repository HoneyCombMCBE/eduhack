#include "SwapChainHook.h"
#include "../GUI/ClickGUI.h"
#include "../GUI/Notifications.h"
#include "../Features/CoordsDisplay.h"
#include "../Features/ArrayList.h"
#include "../Input/KeyInput.h"
#include <d3d11.h>
#include <d3d11on12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <kiero.hpp>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <windows.h>
#include <MinHook.h>
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
extern volatile bool g_disable;

namespace edu::rendering {

static ID3D11Device*        g_d3d11Device  = nullptr;
static ID3D11DeviceContext* g_d3d11Context = nullptr;
static ID3D11On12Device*    g_d3d11on12    = nullptr;
static ID3D12CommandQueue*  g_cmdQueue     = nullptr;
static HWND                 g_hwnd         = nullptr;
static bool                 g_imgui        = false;
static bool                 g_initFailed   = false;
static bool                 g_isD3D12      = false;
static bool                 g_welcomed     = false;
static bool                 g_installed    = false;
static WNDPROC              g_origWndProc  = nullptr;

struct FrameContext {
    ID3D11RenderTargetView* rtv     = nullptr;
    ID3D11Resource*         wrapped = nullptr;
};
static std::vector<FrameContext> g_frames;
static UINT g_bufferCount = 0;

using PresentFn       = HRESULT(*)(IDXGISwapChain*, UINT, UINT);
using ResizeBuffersFn = HRESULT(*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
using ExecuteCommandListsFn = void(*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

static PresentFn             oPresent = nullptr;
static ResizeBuffersFn       oResizeBuffers = nullptr;
static ExecuteCommandListsFn oExecuteCommandLists = nullptr;

static LRESULT CALLBACK hk_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (gui::isOpen()) {
        switch (msg) {
        case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        case WM_CHAR: case WM_UNICHAR:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
        case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
        case WM_INPUT:
            return 0;
        }
    }

    return CallWindowProcA(g_origWndProc, hwnd, msg, wp, lp);
}

static void hk_ExecuteCommandLists(ID3D12CommandQueue* q, UINT n, ID3D12CommandList* const* lists) {
    if (!g_cmdQueue) g_cmdQueue = q;
    oExecuteCommandLists(q, n, lists);
}

static void cleanupFrames() {
    for (auto& f : g_frames) {
        if (f.rtv) { f.rtv->Release(); f.rtv = nullptr; }
        if (f.wrapped) { f.wrapped->Release(); f.wrapped = nullptr; }
    }
    g_frames.clear();
}

static bool createFrameResources(IDXGISwapChain* sc) {
    DXGI_SWAP_CHAIN_DESC desc{};
    sc->GetDesc(&desc);
    g_bufferCount = desc.BufferCount ? desc.BufferCount : 1;
    g_frames.resize(g_bufferCount);

    if (!g_isD3D12) {
        ID3D11Texture2D* buf = nullptr;
        if (FAILED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buf)) || !buf)
            return false;
        g_d3d11Device->CreateRenderTargetView(buf, nullptr, &g_frames[0].rtv);
        buf->Release();
        return g_frames[0].rtv != nullptr;
    }

    for (UINT i = 0; i < g_bufferCount; i++) {
        ID3D12Resource* d3d12Buf = nullptr;
        if (FAILED(sc->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&d3d12Buf)))
            return false;
        D3D11_RESOURCE_FLAGS flags11 = { D3D11_BIND_RENDER_TARGET };
        HRESULT hr = g_d3d11on12->CreateWrappedResource(
            d3d12Buf, &flags11,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT,
            __uuidof(ID3D11Resource), (void**)&g_frames[i].wrapped
        );
        d3d12Buf->Release();
        if (FAILED(hr)) return false;
        ID3D11Texture2D* tex = nullptr;
        g_frames[i].wrapped->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
        if (!tex) return false;
        g_d3d11Device->CreateRenderTargetView(tex, nullptr, &g_frames[i].rtv);
        tex->Release();
        if (!g_frames[i].rtv) return false;
    }
    return true;
}

static void initImGui(IDXGISwapChain* sc) {
    if (g_imgui || g_initFailed) return;

    DXGI_SWAP_CHAIN_DESC desc{};
    if (FAILED(sc->GetDesc(&desc))) return;
    g_hwnd = desc.OutputWindow;

    if (SUCCEEDED(sc->GetDevice(__uuidof(ID3D11Device), (void**)&g_d3d11Device)) && g_d3d11Device) {
        g_isD3D12 = false;
        g_d3d11Device->GetImmediateContext(&g_d3d11Context);
    } else {
        g_d3d11Device = nullptr;
        g_isD3D12 = true;
        if (!g_cmdQueue) return;
        ID3D12Device* d3d12dev = nullptr;
        if (FAILED(sc->GetDevice(__uuidof(ID3D12Device), (void**)&d3d12dev)) || !d3d12dev) {
            g_initFailed = true;
            return;
        }
        IUnknown* queueUnk = g_cmdQueue;
        HRESULT hr = D3D11On12CreateDevice(
            d3d12dev, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr, 0, &queueUnk, 1, 0,
            &g_d3d11Device, &g_d3d11Context, nullptr
        );
        d3d12dev->Release();
        if (FAILED(hr) || !g_d3d11Device) { g_initFailed = true; return; }
        g_d3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&g_d3d11on12);
    }

    if (!createFrameResources(sc)) { g_initFailed = true; return; }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    float fontPx = desc.BufferDesc.Height * 0.028f;
    if (fontPx < 18.0f) fontPx = 18.0f;
    if (fontPx > 48.0f) fontPx = 48.0f;
    ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", fontPx);

    gui::applyStyle();
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_d3d11Device, g_d3d11Context);
    g_origWndProc = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)hk_WndProc);
    g_imgui = true;
}

static HRESULT hk_Present(IDXGISwapChain* sc, UINT sync, UINT flags) {
    if (g_disable) return oPresent(sc, sync, flags);
    initImGui(sc);
    if (!g_imgui) return oPresent(sc, sync, flags);

    UINT idx = 0;
    if (g_isD3D12 && g_frames.size() > 1) {
        IDXGISwapChain3* sc3 = nullptr;
        if (SUCCEEDED(sc->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&sc3))) {
            idx = sc3->GetCurrentBackBufferIndex();
            sc3->Release();
        }
    }
    if (idx >= g_frames.size()) return oPresent(sc, sync, flags);

    auto& frame = g_frames[idx];
    if (!frame.rtv) return oPresent(sc, sync, flags);

    if (g_isD3D12 && g_d3d11on12 && frame.wrapped)
        g_d3d11on12->AcquireWrappedResources(&frame.wrapped, 1);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (!g_welcomed) {
        gui::notifications::notify("heheboi loaded");
        g_welcomed = true;
    }

    {
        static bool rshiftWas = false;
        bool rshiftNow = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
        if (rshiftNow && !rshiftWas) gui::toggle();
        rshiftWas = rshiftNow;

        static bool escWas = false;
        bool escNow = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        if (escNow && !escWas && gui::isOpen()) gui::toggle();
        escWas = escNow;
    }

    {
        static bool prevF7 = false;
        bool f7 = (GetAsyncKeyState(VK_F7) & 0x8000) != 0;
        if (f7 && !prevF7) {
            auto* ci = edu::getClientInstance();
            if (ci) {
                void* gr = ci->getGameRenderer();
                if (gr) {
                    char buf[256];
                    int offsets[] = {0x380, 0x3C0, 0x400, 0x440};
                    const char* names[] = {"0x380", "0x3C0", "0x400", "0x440"};
                    for (int m = 0; m < 4; m++) {
                        float* mat = reinterpret_cast<float*>((char*)gr + offsets[m]);
                        std::snprintf(buf, sizeof(buf), "%s diag: %.3f %.3f %.3f %.3f",
                            names[m], mat[0], mat[5], mat[10], mat[15]);
                        edu::logChat(buf);
                    }
                }
            }
        }
        prevF7 = f7;
    }

    gui::render();
    features::renderCoords();
    features::renderArrayList();
    gui::notifications::render();

    ImGui::Render();
    g_d3d11Context->OMSetRenderTargets(1, &frame.rtv, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (g_isD3D12 && g_d3d11on12 && frame.wrapped) {
        g_d3d11on12->ReleaseWrappedResources(&frame.wrapped, 1);
        g_d3d11Context->Flush();
    }

    return oPresent(sc, sync, flags);
}

static HRESULT hk_ResizeBuffers(IDXGISwapChain* sc, UINT count, UINT w, UINT h,
                                 DXGI_FORMAT fmt, UINT fl) {
    cleanupFrames();
    HRESULT hr = oResizeBuffers(sc, count, w, h, fmt, fl);
    if (g_imgui) createFrameResources(sc);
    return hr;
}

bool isInstalled() { return g_installed; }

void tryLazyInit() {
    if (g_installed) return;
    auto status = kiero::init(kiero::RenderType::Auto);
    if (status != kiero::Status::Success) return;

    kiero::bind<&IDXGISwapChain::Present>(&oPresent, &hk_Present);
    kiero::bind<&IDXGISwapChain::ResizeBuffers>(&oResizeBuffers, &hk_ResizeBuffers);

    if (kiero::getRenderType() == kiero::RenderType::D3D12) {
        auto pExec = kiero::getMethod<&ID3D12CommandQueue::ExecuteCommandLists>();
        if (pExec) {
            MH_CreateHook((void*)pExec, (void*)&hk_ExecuteCommandLists, (void**)&oExecuteCommandLists);
            MH_EnableHook((void*)pExec);
        }
    }

    g_installed = true;
}

bool installSwapChainHook() { tryLazyInit(); return g_installed; }

void removeSwapChainHook() {
    if (g_origWndProc && g_hwnd)
        SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)g_origWndProc);
    if (g_imgui) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
    cleanupFrames();
    if (g_d3d11on12) g_d3d11on12->Release();
    if (g_d3d11Context) g_d3d11Context->Release();
    if (g_d3d11Device) g_d3d11Device->Release();
}

} // namespace edu::rendering
