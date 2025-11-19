/**
 * @file D3DRenderHook_Shared.cpp
 * @brief Shared D3D11 rendering functionality for both DLL and GW2AL modes
 */

#include "D3DRenderHook.h"
#include "../Core/Config.h"
#include "../Core/AppState.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/ImGui/imgui.h"
#include "HookManager.h"
#include "UI/Backend/OverlayWindow.h"

namespace kx::Hooking {

    // Static member initialization
    Present D3DRenderHook::m_pTargetPresent = nullptr;
    Present D3DRenderHook::m_pOriginalPresent = nullptr;
    bool D3DRenderHook::m_isInit = false;
    HWND D3DRenderHook::m_hWindow = NULL;
    ID3D11Device* D3DRenderHook::m_pDevice = nullptr;
    ID3D11DeviceContext* D3DRenderHook::m_pContext = nullptr;
    WNDPROC D3DRenderHook::m_pOriginalWndProc = nullptr;
    AppLifecycleManager* D3DRenderHook::m_pLifecycleManager = nullptr;
    
    // WndProc state initialization
    bool D3DRenderHook::m_rightMouseDown = false;
    bool D3DRenderHook::m_leftMouseDown = false;
    bool D3DRenderHook::m_wasOverImGuiWindow = false;

    bool D3DRenderHook::InitializeFromDevice(ID3D11Device* device, IDXGISwapChain* pSwapChain) {
        if (m_isInit) return true;

        LOG_INFO("[D3DRenderHook] Initializing from provided device (GW2AL mode)");

        if (!device || !pSwapChain) {
            LOG_ERROR("[D3DRenderHook] Null device or swap chain provided during initialization");
            return false;
        }

        try {
            m_pDevice = device;
            // Device reference was incremented by QueryInterface in caller
            // We are responsible for releasing it in CleanupD3DResources()

            m_pDevice->GetImmediateContext(&m_pContext);

            // Get window handle from swap chain description
            DXGI_SWAP_CHAIN_DESC sd;
            HRESULT hr = pSwapChain->GetDesc(&sd);
            if (FAILED(hr)) {
                LOG_ERROR("[D3DRenderHook] Failed to get swap chain description. HRESULT: 0x%08X", hr);
                // Only release context - device reference is owned by caller (GW2AL)
                if (m_pContext) {
                    m_pContext->Release();
                    m_pContext = nullptr;
                }
                m_pDevice = nullptr;  // Just clear the pointer, don't release
                return false;
            }
            m_hWindow = sd.OutputWindow;

            // Note: We no longer create a cached render target view here
            // Instead, we create a fresh RTV every frame in the GW2AL OnPresent callback
            // This ensures proper handling of window resize events

            // Hook the window procedure to handle input for our UI
            m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
            if (!m_pOriginalWndProc) {
                LOG_ERROR("[D3DRenderHook] Failed to hook WndProc in GW2AL mode.");
                CleanupD3DResources(false);
                return false;
            }

            // Initialize ImGui
            if (!OverlayWindow::Initialize(m_pDevice, m_pContext, m_hWindow)) {
                LOG_ERROR("[D3DRenderHook] Failed to initialize ImGui in GW2AL mode.");
                CleanupD3DResources(true);
                return false;
            }

            m_isInit = true;
            LOG_INFO("[D3DRenderHook] Initialized successfully via GW2AL.");
            AppState::Get().SetPresentHookStatus(HookStatus::OK);
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("[D3DRenderHook] Exception during D3D initialization: %s", e.what());
            return false;
        }
        catch (...) {
            LOG_ERROR("[D3DRenderHook] Unknown exception during D3D initialization.");
            return false;
        }
    }

    void D3DRenderHook::OnResize(IDXGISwapChain* pSwapChain) {
        if (!m_isInit) return;

        LOG_INFO("[D3DRenderHook] Handling resize event");
        
        // Note: In DLL mode, we now create a fresh RTV every frame, so no cached RTV to release
        // In GW2AL mode, the RTV is also created per-frame, so this function is mainly for logging
        // The actual resize handling is done automatically by creating fresh RTVs each frame
    }

    void D3DRenderHook::Shutdown() {
        if (m_isInit) {
            // CRITICAL: Disable and remove Present hook BEFORE destroying ImGui
            // This prevents new Present calls from entering while we destroy resources
            if (m_pTargetPresent && m_pOriginalPresent) {
                HookManager::DisableHook(m_pTargetPresent);
                HookManager::RemoveHook(m_pTargetPresent);
                LOG_INFO("[D3DRenderHook] Present hook disabled and removed.");
            }

            // CRITICAL: Restore WndProc BEFORE destroying ImGui to prevent WndProc from calling ImGui during destruction
            if (m_hWindow && m_pOriginalWndProc) {
                SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOriginalWndProc));
                m_pOriginalWndProc = nullptr;
                LOG_INFO("[D3DRenderHook] WndProc restored before ImGui shutdown.");
            }

            // Now safe to shutdown ImGui
            OverlayWindow::Shutdown();
            LOG_INFO("[D3DRenderHook] ImGui shutdown.");
        }

        // Clean up remaining D3D resources (WndProc already restored above)
        CleanupD3DResources(false);
        LOG_INFO("[D3DRenderHook] D3D resources released.");

        // Reset WndProc state
        m_rightMouseDown = false;
        m_leftMouseDown = false;
        m_wasOverImGuiWindow = false;

        m_isInit = false;
        m_pOriginalPresent = nullptr;
        m_pTargetPresent = nullptr;
        AppState::Get().SetPresentHookStatus(HookStatus::Unknown);
    }

    bool D3DRenderHook::IsInitialized() {
        return m_isInit;
    }

    void D3DRenderHook::SetLifecycleManager(AppLifecycleManager* lifecycleManager) {
        m_pLifecycleManager = lifecycleManager;
    }

    void D3DRenderHook::CleanupD3DResources(bool includeWndProc) {
        if (includeWndProc && m_hWindow && m_pOriginalWndProc) {
            SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOriginalWndProc));
            m_pOriginalWndProc = nullptr;
        }

        // Note: RTVs are created and released per-frame in both DLL and GW2AL modes
        // No cached RTV to release here
        if (m_pContext) { 
            m_pContext->Release(); 
            m_pContext = nullptr; 
        }
        if (m_pDevice) { 
            m_pDevice->Release(); 
            m_pDevice = nullptr; 
        }
        if (includeWndProc) {
            m_hWindow = NULL;
        }
    }

} // namespace kx::Hooking
