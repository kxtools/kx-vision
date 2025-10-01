/**
 * @file D3DRenderHook_Shared.cpp
 * @brief Shared D3D11 rendering functionality used by both DLL and GW2AL modes
 * 
 * This file contains code that is common to both build modes:
 * - Device initialization from swap chain (GW2AL mode)
 * - Resize event handling
 * - Shutdown and cleanup
 * - State accessors
 */

#include "D3DRenderHook.h"
#include "../Core/Config.h"
#include "../Core/AppState.h"
#include "../Utils/DebugLogger.h"
#include "../Rendering/ImGuiManager.h"
#include "../../libs/ImGui/imgui.h"
#include "../../libs/ImGui/imgui_impl_dx11.h"

namespace kx::Hooking {

    // Initialize static members
    Present D3DRenderHook::m_pTargetPresent = nullptr;
    Present D3DRenderHook::m_pOriginalPresent = nullptr;
    bool D3DRenderHook::m_isInit = false;
    HWND D3DRenderHook::m_hWindow = NULL;
    ID3D11Device* D3DRenderHook::m_pDevice = nullptr;
    ID3D11DeviceContext* D3DRenderHook::m_pContext = nullptr;
    ID3D11RenderTargetView* D3DRenderHook::m_pMainRenderTargetView = nullptr;
    WNDPROC D3DRenderHook::m_pOriginalWndProc = nullptr;
    kx::AppLifecycleManager* D3DRenderHook::m_pLifecycleManager = nullptr;

    bool D3DRenderHook::InitializeFromDevice(ID3D11Device* device, IDXGISwapChain* pSwapChain) {
        if (m_isInit) return true;

        LOG_INFO("[D3DRenderHook] Initializing from provided device (GW2AL mode)");

        m_pDevice = device;
        m_pDevice->GetImmediateContext(&m_pContext);

        // Get window handle from swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);
        m_hWindow = sd.OutputWindow;

        // Create the render target view
        ID3D11Texture2D* pBackBuffer = nullptr;
        if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) {
            m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
            pBackBuffer->Release();
        } else {
            LOG_ERROR("[D3DRenderHook] Failed to get back buffer from GW2AL swap chain.");
            return false;
        }

        // Hook WndProc (still needed for input handling)
        m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
        if (!m_pOriginalWndProc) {
            LOG_ERROR("[D3DRenderHook] Failed to hook WndProc in GW2AL mode.");
            if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
            return false;
        }

        // Initialize ImGui
        if (!ImGuiManager::Initialize(m_pDevice, m_pContext, m_hWindow)) {
            LOG_ERROR("[D3DRenderHook] Failed to initialize ImGui in GW2AL mode.");
            SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)m_pOriginalWndProc); // Restore WndProc
            if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
            return false;
        }

        m_isInit = true;
        LOG_INFO("[D3DRenderHook] Initialized successfully via GW2AL.");
        kx::AppState::Get().SetPresentHookStatus(kx::HookStatus::OK);
        return true;
    }

    void D3DRenderHook::OnResize(IDXGISwapChain* pSwapChain) {
        if (!m_isInit) return;

        LOG_INFO("[D3DRenderHook] Handling resize event");

        // Release the old render target view
        if (m_pMainRenderTargetView) {
            m_pMainRenderTargetView->Release();
            m_pMainRenderTargetView = nullptr;
        }
        ImGui_ImplDX11_InvalidateDeviceObjects();

        // The RTV will be recreated on the next Present call, but we must
        // tell ImGui about the new size immediately.
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)sd.BufferDesc.Width, (float)sd.BufferDesc.Height);

        ImGui_ImplDX11_CreateDeviceObjects();
        LOG_INFO("[D3DRenderHook] Swap chain resized to %dx%d.", sd.BufferDesc.Width, sd.BufferDesc.Height);
    }

    void D3DRenderHook::Shutdown() {
        // Restore original WndProc FIRST
        if (m_hWindow && m_pOriginalWndProc) {
            SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOriginalWndProc));
            m_pOriginalWndProc = nullptr;
            LOG_INFO("[D3DRenderHook] Restored original WndProc.");
        }

        // Shutdown ImGui
        if (m_isInit) {
            ImGuiManager::Shutdown();
            LOG_INFO("[D3DRenderHook] ImGui shutdown.");
        }

        // Release D3D resources
        if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
        if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
        if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
        LOG_INFO("[D3DRenderHook] D3D resources released.");

        m_isInit = false;
        m_hWindow = NULL;
        m_pOriginalPresent = nullptr;
        kx::AppState::Get().SetPresentHookStatus(kx::HookStatus::Unknown);
    }

    bool D3DRenderHook::IsInitialized() {
        return m_isInit;
    }

    void D3DRenderHook::SetLifecycleManager(kx::AppLifecycleManager* lifecycleManager) {
        m_pLifecycleManager = lifecycleManager;
    }

} // namespace kx::Hooking
