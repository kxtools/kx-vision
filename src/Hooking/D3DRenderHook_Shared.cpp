/**
 * @file D3DRenderHook_Shared.cpp
 * @brief Shared D3D11 rendering functionality for both DLL and GW2AL modes
 */

#include "D3DRenderHook.h"
#include "../Core/AppLifecycleManager.h"
#include "../Utils/DebugLogger.h"
#include "HookManager.h"
#include "UI/Backend/OverlayWindow.h"

namespace kx::Hooking {

    // Static member initialization
    Present D3DRenderHook::m_pTargetPresent = nullptr;
    Present D3DRenderHook::m_pOriginalPresent = nullptr;
    bool D3DRenderHook::m_isInit = false;
    bool D3DRenderHook::m_presentHookInstalled = false;
    HookHandle D3DRenderHook::m_presentHook{};
    HWND D3DRenderHook::m_hWindow = NULL;
    ID3D11Device* D3DRenderHook::m_pDevice = nullptr;
    ID3D11DeviceContext* D3DRenderHook::m_pContext = nullptr;
    AppLifecycleManager* D3DRenderHook::m_pLifecycleManager = nullptr;

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

            if (!InstallWndProc(m_hWindow)) {
                LOG_ERROR("[D3DRenderHook] Failed to hook WndProc in GW2AL mode.");
                CleanupD3DResources(false);
                return false;
            }

            // Initialize ImGui
            if (!m_pLifecycleManager ||
                !OverlayWindow::Initialize(m_pDevice, m_pContext, m_hWindow,
                                           m_pLifecycleManager->GetSettings())) {
                LOG_ERROR("[D3DRenderHook] Failed to initialize ImGui in GW2AL mode.");
                CleanupD3DResources(true);
                return false;
            }

            m_isInit = true;
            LOG_INFO("[D3DRenderHook] Initialized successfully via GW2AL.");
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
        // Hook installation and ImGui initialization are separate stages. A shutdown
        // before first Present must still detach the installed callback.
        if (m_presentHookInstalled && m_pTargetPresent) {
            HookManager::DisableAndRemove(m_presentHook);
            m_presentHookInstalled = false;
            LOG_INFO("[D3DRenderHook] Present hook disabled and removed.");
        }
        if (m_isInit) {
            UninstallWndProc();
            LOG_INFO("[D3DRenderHook] WndProc restored before ImGui shutdown.");

            // Now safe to shutdown ImGui
            OverlayWindow::Shutdown();
            LOG_INFO("[D3DRenderHook] ImGui shutdown.");
        }

        // Clean up remaining D3D resources (WndProc already restored above)
        CleanupD3DResources(false);
        LOG_INFO("[D3DRenderHook] D3D resources released.");

        m_isInit = false;
        m_presentHookInstalled = false;
        m_pOriginalPresent = nullptr;
        m_pTargetPresent = nullptr;
    }

    bool D3DRenderHook::IsInitialized() {
        return m_isInit;
    }

    void D3DRenderHook::SetLifecycleManager(AppLifecycleManager* lifecycleManager) {
        m_pLifecycleManager = lifecycleManager;
    }

    void D3DRenderHook::CleanupD3DResources(bool includeWndProc) {
        if (includeWndProc)
            UninstallWndProc();

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
