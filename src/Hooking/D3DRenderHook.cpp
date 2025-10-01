#include "D3DRenderHook.h"

#include <windowsx.h>

#include "../Core/AppState.h"         // For UI visibility state and shutdown coordination
#include "../Core/AppLifecycleManager.h" // For accessing Camera and MumbleLink data
#include "../Utils/DebugLogger.h"
#include "HookManager.h"      // To create/remove the hook
#include "ImGuiManager.h"     // To initialize and render ImGui
#include "../../libs/ImGui/imgui.h"

// Declare the external ImGui Win32 handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

    bool D3DRenderHook::Initialize() {
        if (!FindPresentPointer()) {
            LOG_ERROR("[D3DRenderHook] Failed to find Present pointer.");
            return false;
        }

        // Use HookManager to create the hook, but don't enable yet.
        // Enable happens on first DetourPresent call if needed, or enable here?
        // Let's create and enable it immediately.
        if (!HookManager::CreateHook(m_pTargetPresent, DetourPresent, reinterpret_cast<LPVOID*>(&m_pOriginalPresent))) {
            LOG_ERROR("[D3DRenderHook] Failed to create Present hook via HookManager.");
            return false;
        }
        if (!HookManager::EnableHook(m_pTargetPresent)) {
            LOG_ERROR("[D3DRenderHook] Failed to enable Present hook via HookManager.");
            // HookManager::RemoveHook(m_pTargetPresent); // Attempt cleanup if enable fails
            return false;
        }

        LOG_INFO("[D3DRenderHook] Present hook created and enabled.");
        kx::AppState::Get().SetPresentHookStatus(kx::HookStatus::OK); // Update hook status via singleton
        return true;
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

        // Request HookManager to disable/remove the hook (usually done in HookManager::Shutdown)
        // HookManager::DisableHook(m_pTargetPresent); // Optionally disable explicitly
        // HookManager::RemoveHook(m_pTargetPresent); // Optionally remove explicitly

        m_isInit = false;
        m_hWindow = NULL;
        m_pOriginalPresent = nullptr;
        kx::AppState::Get().SetPresentHookStatus(kx::HookStatus::Unknown); // Reset status via singleton
    }

    bool D3DRenderHook::IsInitialized() {
        return m_isInit;
    }

    void D3DRenderHook::SetLifecycleManager(kx::AppLifecycleManager* lifecycleManager) {
        m_pLifecycleManager = lifecycleManager;
    }

    bool D3DRenderHook::FindPresentPointer() {
        // This logic remains complex but necessary. Keep it encapsulated here.
        const wchar_t* DUMMY_WNDCLASS_NAME = L"KxDummyWindowPresent"; // Use unique name
        HWND dummy_hwnd = NULL;
        WNDCLASSEXW wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProcW, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, DUMMY_WNDCLASS_NAME, NULL };

        if (!RegisterClassExW(&wc)) {
            if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) { // Check if it failed for a real reason
                LOG_ERROR("[D3DRenderHook] Failed to register dummy window class. Error: %d", GetLastError());
                return false;
            }
        }

        dummy_hwnd = CreateWindowW(DUMMY_WNDCLASS_NAME, NULL, WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, wc.hInstance, NULL);
        if (!dummy_hwnd) {
            LOG_ERROR("[D3DRenderHook] Failed to create dummy window. Error: %d", GetLastError());
            UnregisterClassW(DUMMY_WNDCLASS_NAME, wc.hInstance);
            return false;
        }

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = dummy_hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        IDXGISwapChain* pSwapChain = nullptr;
        ID3D11Device* pDevice = nullptr; // Temporary device
        bool success = false;

        const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2,
            D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, nullptr, nullptr);

        if (hr == S_OK) {
            void** vTable = *reinterpret_cast<void***>(pSwapChain);
            m_pTargetPresent = reinterpret_cast<Present>(vTable[8]); // Store the found pointer
            pSwapChain->Release();
            pDevice->Release();
            success = true;
            LOG_INFO("[D3DRenderHook] Found Present pointer at: 0x%p", m_pTargetPresent);
        }
        else {
            LOG_ERROR("[D3DRenderHook] D3D11CreateDeviceAndSwapChain failed (HRESULT: 0x%X)", hr);
            if (pSwapChain) pSwapChain->Release();
            if (pDevice) pDevice->Release();
        }

        if (dummy_hwnd) DestroyWindow(dummy_hwnd);
        UnregisterClassW(DUMMY_WNDCLASS_NAME, wc.hInstance);
        return success;
    }


    HRESULT __stdcall D3DRenderHook::DetourPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        // Check the shutdown flag FIRST. If set, immediately call original and exit.
        // This prevents accessing potentially destroyed resources (ImGui context, etc.)
        if (kx::AppState::Get().IsShuttingDown()) {
            return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
        }

        // One-time initialization of D3D resources and ImGui
        if (!m_isInit) {
            if (!InitializeD3DResources(pSwapChain)) {
                return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
            }
        }

        // Per-frame logic
        if (m_isInit) {
            HandleInput();
            RenderFrame();
        }

        // Call original Present function
        return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
    }

    bool D3DRenderHook::InitializeD3DResources(IDXGISwapChain* pSwapChain) {
        // Attempt to get D3D device from swap chain
        if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&m_pDevice)))) {
            LOG_ERROR("[D3DRenderHook] Failed to get D3D device from swap chain");
            return false;
        }

        // Get immediate context
        m_pDevice->GetImmediateContext(&m_pContext);

        // Get window handle from swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);
        m_hWindow = sd.OutputWindow;

        // Create render target view from back buffer
        ID3D11Texture2D* pBackBuffer = nullptr;
        if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)))) {
            LOG_ERROR("[D3DRenderHook] Failed to get back buffer");
            // Cleanup on failure
            if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
            if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
            m_hWindow = NULL;
            return false;
        }

        m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
        pBackBuffer->Release();

        // Hook WndProc
        m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
        if (!m_pOriginalWndProc) {
            LOG_ERROR("[D3DRenderHook] Failed to hook WndProc");
            // Cleanup on failure
            if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
            if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
            if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
            m_hWindow = NULL;
            return false;
        }

        // Initialize ImGui
        if (!ImGuiManager::Initialize(m_pDevice, m_pContext, m_hWindow)) {
            LOG_ERROR("[D3DRenderHook] Failed to initialize ImGui");
            // Restore WndProc on failure
            SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOriginalWndProc));
            m_pOriginalWndProc = nullptr;
            // Cleanup D3D resources
            if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
            if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
            if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
            m_hWindow = NULL;
            return false;
        }

        m_isInit = true;
        LOG_INFO("[D3DRenderHook] D3D resources and ImGui initialized successfully");
        return true;
    }

    void D3DRenderHook::HandleInput() {
        // Handle INSERT key toggle for UI visibility
        static bool lastToggleKeyState = false;
        bool currentToggleKeyState = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        
        if (currentToggleKeyState && !lastToggleKeyState) {
            auto& settings = kx::AppState::Get().GetSettings();
            settings.showVisionWindow = !settings.showVisionWindow;
        }
        
        lastToggleKeyState = currentToggleKeyState;
    }

    void D3DRenderHook::RenderFrame() {
        // Double-check shutdown flag and ImGui context before rendering
        if (kx::AppState::Get().IsShuttingDown() || ImGui::GetCurrentContext() == nullptr) {
            return;
        }

        // Check if we have a lifecycle manager to get game state from
        if (!m_pLifecycleManager) {
            OutputDebugStringA("[D3DRenderHook::RenderFrame] No lifecycle manager set\n");
            return;
        }

        try {
            // Get game state from lifecycle manager
            kx::Camera& camera = m_pLifecycleManager->GetCamera();
            kx::MumbleLinkManager& mumbleLinkManager = m_pLifecycleManager->GetMumbleLinkManager();
            
            // Update game state every frame (not just every 100ms in lifecycle manager)
            mumbleLinkManager.Update();
            const kx::MumbleLinkData* mumbleLinkData = mumbleLinkManager.GetData();
            camera.Update(mumbleLinkData, m_hWindow);
            
            // Get display size
            ImGuiIO& io = ImGui::GetIO();
            
            ImGuiManager::NewFrame();
            ImGuiManager::RenderUI(camera, mumbleLinkManager, mumbleLinkData, 
                                   m_hWindow, io.DisplaySize.x, io.DisplaySize.y);
            ImGuiManager::Render(m_pContext, m_pMainRenderTargetView);
        }
        catch (const std::exception& e) {
            OutputDebugStringA("[D3DRenderHook::RenderFrame] ImGui Exception: ");
            OutputDebugStringA(e.what());
            OutputDebugStringA("\n");
        }
        catch (...) {
            OutputDebugStringA("[D3DRenderHook::RenderFrame] Unknown ImGui Exception\n");
        }
    }

    LRESULT __stdcall D3DRenderHook::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        // Track mouse buttons
        static bool rightMouseDown = false;
        static bool leftMouseDown = false;
        static bool wasOverImGuiWindow = false;

        // Update mouse button states
        if (uMsg == WM_RBUTTONDOWN) rightMouseDown = true;
        else if (uMsg == WM_RBUTTONUP) rightMouseDown = false;
        else if (uMsg == WM_LBUTTONDOWN) leftMouseDown = true;
        else if (uMsg == WM_LBUTTONUP) leftMouseDown = false;

        // Only process ImGui input if overlay is visible
        if (m_isInit && kx::AppState::Get().GetSettings().showVisionWindow) {
            // First, check if the mouse is over an ImGui window without changing input state
            bool isOverImGuiWindow = false;

            // For mouse events, temporarily pass to ImGui to check if mouse is over a window
            if (uMsg == WM_MOUSEMOVE) {
                // Check if mouse is over any ImGui window (without handling the input)
                isOverImGuiWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
                    ImGui::IsAnyItemHovered();

                // Store this for when other messages come in
                wasOverImGuiWindow = isOverImGuiWindow;
            }
            else {
                // For non-mousemove events, use the last known hover state
                isOverImGuiWindow = wasOverImGuiWindow;
            }

            // Special handling for left mouse button for camera rotation
            if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP) && !isOverImGuiWindow) {
                // If LMB and not over ImGui, pass directly to game without ImGui processing
                return CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam);
            }

            // Handle other inputs normally - if RMB isn't down OR mouse is over ImGui window
            if (!rightMouseDown || isOverImGuiWindow) {
                // Process through ImGui
                bool handled = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

                // Get ImGui IO
                ImGuiIO& io = ImGui::GetIO();

                // If ImGui wants the input, don't pass to the game
                if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
                    return 1; // Handled by ImGui
                }
            }
        }

        // Pass the message to the original game window procedure
        return m_pOriginalWndProc ? CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam)
            : DefWindowProc(hWnd, uMsg, wParam, lParam);
    }


} // namespace kx::Hooking

