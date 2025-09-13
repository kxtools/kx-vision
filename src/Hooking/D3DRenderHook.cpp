#include "D3DRenderHook.h"

#include <iostream>           // Replace with logging
#include <windowsx.h>

#include "../Core/AppState.h"         // For UI visibility state and shutdown coordination
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

    bool D3DRenderHook::Initialize() {
        if (!FindPresentPointer()) {
            std::cerr << "[D3DRenderHook] ERROR: Failed to find Present pointer." << std::endl;
            return false;
        }

        // Use HookManager to create the hook, but don't enable yet.
        // Enable happens on first DetourPresent call if needed, or enable here?
        // Let's create and enable it immediately.
        if (!HookManager::CreateHook(m_pTargetPresent, DetourPresent, reinterpret_cast<LPVOID*>(&m_pOriginalPresent))) {
            std::cerr << "[D3DRenderHook] Failed to create Present hook via HookManager." << std::endl;
            return false;
        }
        if (!HookManager::EnableHook(m_pTargetPresent)) {
            std::cerr << "[D3DRenderHook] Failed to enable Present hook via HookManager." << std::endl;
            // HookManager::RemoveHook(m_pTargetPresent); // Attempt cleanup if enable fails
            return false;
        }

        std::cout << "[D3DRenderHook] Present hook created and enabled." << std::endl;
        kx::AppState::Get().SetPresentHookStatus(kx::HookStatus::OK); // Update hook status via singleton
        return true;
    }

    void D3DRenderHook::Shutdown() {
        // Restore original WndProc FIRST
        if (m_hWindow && m_pOriginalWndProc) {
            SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOriginalWndProc));
            m_pOriginalWndProc = nullptr;
            std::cout << "[D3DRenderHook] Restored original WndProc." << std::endl;
        }

        // Shutdown ImGui
        if (m_isInit) {
            ImGuiManager::Shutdown();
            std::cout << "[D3DRenderHook] ImGui shutdown." << std::endl;
        }

        // Release D3D resources
        if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
        if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
        if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
        std::cout << "[D3DRenderHook] D3D resources released." << std::endl;

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


    bool D3DRenderHook::FindPresentPointer() {
        // This logic remains complex but necessary. Keep it encapsulated here.
        const wchar_t* DUMMY_WNDCLASS_NAME = L"KxDummyWindowPresent"; // Use unique name
        HWND dummy_hwnd = NULL;
        WNDCLASSEXW wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProcW, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, DUMMY_WNDCLASS_NAME, NULL };

        if (!RegisterClassExW(&wc)) {
            if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) { // Check if it failed for a real reason
                std::cerr << "[D3DRenderHook] Failed to register dummy window class. Error: " << GetLastError() << std::endl;
                return false;
            }
        }

        dummy_hwnd = CreateWindowW(DUMMY_WNDCLASS_NAME, NULL, WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, wc.hInstance, NULL);
        if (!dummy_hwnd) {
            std::cerr << "[D3DRenderHook] Failed to create dummy window. Error: " << GetLastError() << std::endl;
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
            std::cout << "[D3DRenderHook] Found Present pointer at: 0x" << std::hex << m_pTargetPresent << std::dec << std::endl;
        }
        else {
            std::cerr << "[D3DRenderHook] D3D11CreateDeviceAndSwapChain failed (HRESULT: 0x" << std::hex << hr << std::dec << ")" << std::endl;
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
        if (kx::AppState::Get().IsShuttingDown()) { // Use AppState singleton method
            return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
        }

        if (!m_isInit) {
            // Attempt to initialize D3D resources and ImGui using the game's swapchain/device
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&m_pDevice)))) {
                m_pDevice->GetImmediateContext(&m_pContext);
                DXGI_SWAP_CHAIN_DESC sd;
                pSwapChain->GetDesc(&sd);
                m_hWindow = sd.OutputWindow;

                ID3D11Texture2D* pBackBuffer = nullptr;
                if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)))) {
                    m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
                    pBackBuffer->Release();
                }
                else {
                    std::cerr << "[D3DRenderHook] Failed to get back buffer." << std::endl;
                    // Release potentially acquired resources on partial failure
                    if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
                    if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
                    m_hWindow = NULL;
                    return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
                }

                // Hook WndProc now that we have the window handle
                m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
                if (!m_pOriginalWndProc) {
                    std::cerr << "[D3DRenderHook] Failed to hook WndProc." << std::endl;
                    // Clean up D3D resources if WndProc hook fails
                    if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
                    if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
                    if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
                    m_hWindow = NULL;
                    return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
                }

                // Initialize ImGui
                if (!ImGuiManager::Initialize(m_pDevice, m_pContext, m_hWindow)) {
                    std::cerr << "[D3DRenderHook] Failed to initialize ImGui." << std::endl;
                    // Restore WndProc if ImGui init fails
                    SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOriginalWndProc));
                    m_pOriginalWndProc = nullptr;
                    // Clean up D3D resources
                    if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
                    if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
                    if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
                    m_hWindow = NULL;
                    // Continue without ImGui
                }
                else {
                    m_isInit = true; // Full initialization successful
                    std::cout << "[D3DRenderHook] ImGui Initialized." << std::endl;
                }
            }
            else {
                // Failed to get device, cannot initialize yet. Call original.
                return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
            }
        }

        // --- Hotkey / Frame Logic ---
		// Hotkey check should ideally also check m_isInit?
        if (m_isInit) { // Only process hotkeys/UI if fully initialized
            static bool lastToggleKeyState = false;
            bool currentToggleKeyState = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
            if (currentToggleKeyState && !lastToggleKeyState) {
                auto& settings = kx::AppState::Get().GetSettings();
                settings.showVisionWindow = !settings.showVisionWindow;
            }
            lastToggleKeyState = currentToggleKeyState;

            // --- Render ImGui overlay ---
            // Add ANOTHER check for shutdown flag right before ImGui calls
            // AND ensure ImGui context still exists (check GImGui != nullptr)
            if (!kx::AppState::Get().IsShuttingDown() && ImGui::GetCurrentContext() != nullptr)
            {
                try { // Optional: Add try-catch around ImGui calls during shutdown race possibility
                    ImGuiManager::NewFrame();
                    ImGuiManager::RenderUI(); // Contains ImGui::Begin/End
                    ImGuiManager::Render(m_pContext, m_pMainRenderTargetView);
                }
                catch (const std::exception& e) {
                    OutputDebugStringA("[DetourPresent] ImGui Exception during Render: ");
                    OutputDebugStringA(e.what());
                    OutputDebugStringA("\n");
                }
                catch (...)
                {
                    OutputDebugStringA("[DetourPresent] Unknown ImGui Exception during Render.\n");
                }
            }
            // --- End Render ImGui ---
        }
        // --- End Hotkey / Frame Logic ---

        // Call original Present function - ensure it's valid
        return m_pOriginalPresent ? m_pOriginalPresent(pSwapChain, SyncInterval, Flags) : E_FAIL;
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

