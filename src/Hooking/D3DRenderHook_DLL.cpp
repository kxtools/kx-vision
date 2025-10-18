/**
 * @file D3DRenderHook_DLL.cpp
 * @brief DLL injection mode specific functionality
 * 
 * This file contains DLL-mode specific code:
 * - Finding the Present function pointer via dummy swap chain
 * - Creating and enabling the Present hook with MinHook
 * - DetourPresent implementation with per-frame rendering
 * - Complex WndProc for handling camera rotation conflicts
 * 
 * This file is only compiled when GW2AL_BUILD is NOT defined.
 */

#include "D3DRenderHook.h"

#include "Config.h"
#ifndef GW2AL_BUILD // Only compile this file in DLL mode

#include <windowsx.h>
#include "../Core/AppState.h"
#include "../Core/AppLifecycleManager.h"
#include "../Utils/DebugLogger.h"
#include "HookManager.h"
#include "../Rendering/ImGui/ImGuiManager.h"
#include "../../libs/ImGui/imgui.h"

// Declare the external ImGui Win32 handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace kx::Hooking {

    bool D3DRenderHook::Initialize() {
        if (!FindPresentPointer()) {
            LOG_ERROR("[D3DRenderHook] Failed to find Present pointer.");
            return false;
        }

        // Create and enable the Present hook via HookManager
        if (!HookManager::CreateHook(m_pTargetPresent, DetourPresent, reinterpret_cast<LPVOID*>(&m_pOriginalPresent))) {
            LOG_ERROR("[D3DRenderHook] Failed to create Present hook via HookManager.");
            return false;
        }
        if (!HookManager::EnableHook(m_pTargetPresent)) {
            LOG_ERROR("[D3DRenderHook] Failed to enable Present hook via HookManager.");
            return false;
        }

        LOG_INFO("[D3DRenderHook] Present hook created and enabled.");
        AppState::Get().SetPresentHookStatus(HookStatus::OK);
        return true;
    }

    bool D3DRenderHook::FindPresentPointer() {
        const wchar_t* DUMMY_WNDCLASS_NAME = L"KxDummyWindowPresent";
        HWND dummy_hwnd = NULL;
        WNDCLASSEXW wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProcW, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, DUMMY_WNDCLASS_NAME, NULL };

        if (!RegisterClassExW(&wc)) {
            if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
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
        ID3D11Device* pDevice = nullptr;
        bool success = false;

        const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2,
            D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, nullptr, nullptr);

        if (hr == S_OK) {
            void** vTable = *reinterpret_cast<void***>(pSwapChain);
            m_pTargetPresent = reinterpret_cast<Present>(vTable[8]);
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
        // Check the shutdown flag FIRST
        if (AppState::Get().IsShuttingDown()) {
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
            if (m_pMainRenderTargetView) { m_pMainRenderTargetView->Release(); m_pMainRenderTargetView = nullptr; }
            if (m_pContext) { m_pContext->Release(); m_pContext = nullptr; }
            if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
            m_hWindow = NULL;
            return false;
        }

        // Initialize ImGui
        if (!ImGuiManager::Initialize(m_pDevice, m_pContext, m_hWindow)) {
            LOG_ERROR("[D3DRenderHook] Failed to initialize ImGui");
            SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOriginalWndProc));
            m_pOriginalWndProc = nullptr;
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

    void D3DRenderHook::RenderFrame() {
        // Double-check shutdown flag and ImGui context before rendering
        if (AppState::Get().IsShuttingDown() || ImGui::GetCurrentContext() == nullptr) {
            return;
        }

        // Check if we have a lifecycle manager to get game state from
        if (!m_pLifecycleManager) {
            OutputDebugStringA("[D3DRenderHook::RenderFrame] No lifecycle manager set\n");
            return;
        }

        try {
            // Get display size
            ImGuiIO& io = ImGui::GetIO();
            
            // === Centralized per-frame tick (update + render) ===
            m_pLifecycleManager->RenderTick(m_hWindow, io.DisplaySize.x, io.DisplaySize.y, 
                                            m_pContext, m_pMainRenderTargetView);
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
        // Complex WndProc for DLL injection mode - handle camera rotation conflicts
        static bool rightMouseDown = false;
        static bool leftMouseDown = false;
        static bool wasOverImGuiWindow = false;

        // Update mouse button states
        if (uMsg == WM_RBUTTONDOWN) rightMouseDown = true;
        else if (uMsg == WM_RBUTTONUP) rightMouseDown = false;
        else if (uMsg == WM_LBUTTONDOWN) leftMouseDown = true;
        else if (uMsg == WM_LBUTTONUP) leftMouseDown = false;

        // Only process ImGui input if overlay is visible
        if (m_isInit && AppState::Get().IsVisionWindowOpen()) {
            // Check if the mouse is over an ImGui window
            bool isOverImGuiWindow = false;

            if (uMsg == WM_MOUSEMOVE) {
                isOverImGuiWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
                    ImGui::IsAnyItemHovered();
                wasOverImGuiWindow = isOverImGuiWindow;
            }
            else {
                isOverImGuiWindow = wasOverImGuiWindow;
            }

            // Special handling for left mouse button for camera rotation
            if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP) && !isOverImGuiWindow) {
                // If LMB and not over ImGui, pass directly to game
                return CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam);
            }

            // Handle other inputs - if RMB isn't down OR mouse is over ImGui window
            if (!rightMouseDown || isOverImGuiWindow) {
                ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                ImGuiIO& io = ImGui::GetIO();

                // If ImGui wants the input, don't pass to the game
                if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
                    return 1;
                }
            }
        }

        // Pass to original game window procedure
        return m_pOriginalWndProc ? CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam)
            : DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

} // namespace kx::Hooking

#endif // !GW2AL_BUILD
