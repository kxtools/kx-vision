/**
 * @file D3DRenderHook_Shared.cpp
 * @brief Shared D3D11 rendering functionality for both DLL and GW2AL modes
 */

#include "D3DRenderHook.h"
#include "../Core/Config.h"
#include "../Core/AppState.h"
#include "../Utils/DebugLogger.h"
#include "../Rendering/ImGui/ImGuiManager.h"
#include "../../libs/ImGui/imgui.h"

namespace kx::Hooking {

    // Static member initialization
    Present D3DRenderHook::m_pTargetPresent = nullptr;
    Present D3DRenderHook::m_pOriginalPresent = nullptr;
    bool D3DRenderHook::m_isInit = false;
    HWND D3DRenderHook::m_hWindow = NULL;
    ID3D11Device* D3DRenderHook::m_pDevice = nullptr;
    ID3D11DeviceContext* D3DRenderHook::m_pContext = nullptr;
    ID3D11RenderTargetView* D3DRenderHook::m_pMainRenderTargetView = nullptr;
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
            // Increment reference count - Shutdown() will call Release()
            m_pDevice->AddRef();

            m_pDevice->GetImmediateContext(&m_pContext);

            // Get window handle from swap chain description
            DXGI_SWAP_CHAIN_DESC sd;
            HRESULT hr = pSwapChain->GetDesc(&sd);
            if (FAILED(hr)) {
                LOG_ERROR("[D3DRenderHook] Failed to get swap chain description. HRESULT: 0x%08X", hr);
                // Clean up resources before returning
                if (m_pContext) {
                    m_pContext->Release();
                    m_pContext = nullptr;
                }
                if (m_pDevice) {
                    m_pDevice->Release();
                    m_pDevice = nullptr;
                }
                return false;
            }
            m_hWindow = sd.OutputWindow;

            // Create the render target view (RTV) for rendering our UI
            ID3D11Texture2D* pBackBuffer = nullptr;
            hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
            if (FAILED(hr) || !pBackBuffer) {
                LOG_ERROR("[D3DRenderHook] Failed to get back buffer from swap chain. HRESULT: 0x%08X", hr);
                return false;
            }

            hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
            pBackBuffer->Release();

            if (FAILED(hr) || !m_pMainRenderTargetView) {
                LOG_ERROR("[D3DRenderHook] Failed to create render target view. HRESULT: 0x%08X", hr);
                CleanupD3DResources(false);
                return false;
            }

            // Hook the window procedure to handle input for our UI
            m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
            if (!m_pOriginalWndProc) {
                LOG_ERROR("[D3DRenderHook] Failed to hook WndProc in GW2AL mode.");
                CleanupD3DResources(false);
                return false;
            }

            // Initialize ImGui
            if (!ImGuiManager::Initialize(m_pDevice, m_pContext, m_hWindow)) {
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

        // Release the old cached render target view if it exists
        // Note: In GW2AL mode, the RTV is recreated per-frame in OnPresent callback
        // In DLL mode, it would need to be recreated here or on next Present call
        if (m_pMainRenderTargetView) {
            m_pMainRenderTargetView->Release();
            m_pMainRenderTargetView = nullptr;
        }

        LOG_INFO("[D3DRenderHook] Cached RTV released, will be recreated on next frame");
    }

    void D3DRenderHook::Shutdown() {
        // Shutdown ImGui first
        if (m_isInit) {
            ImGuiManager::Shutdown();
            LOG_INFO("[D3DRenderHook] ImGui shutdown.");
        }

        // Clean up all D3D resources and restore WndProc
        CleanupD3DResources(true);
        LOG_INFO("[D3DRenderHook] D3D resources released.");

        // Reset WndProc state
        m_rightMouseDown = false;
        m_leftMouseDown = false;
        m_wasOverImGuiWindow = false;

        m_isInit = false;
        m_pOriginalPresent = nullptr;
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

        if (m_pMainRenderTargetView) { 
            m_pMainRenderTargetView->Release(); 
            m_pMainRenderTargetView = nullptr; 
        }
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
