/**
 * @file D3DRenderHook_DLL.cpp
 * @brief DLL injection mode specific functionality
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
#include "ImGui/imgui_internal.h"

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
        
        // Initialize WndProc state
        m_rightMouseDown = false;
        m_leftMouseDown = false;
        m_wasOverImGuiWindow = false;
        
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
            m_pTargetPresent = reinterpret_cast<Present>(vTable[VTableIndices::DXGI_PRESENT]);
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
            // Create a fresh render target view every frame (like GW2AL mode)
            // This ensures we always have a valid RTV even after resize
            ID3D11Texture2D* pBackBuffer = nullptr;
            ID3D11RenderTargetView* frameRenderTargetView = nullptr;
            
            if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) {
                HRESULT hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &frameRenderTargetView);
                pBackBuffer->Release();
                
                if (SUCCEEDED(hr) && frameRenderTargetView) {
                    // Get display size from swap chain
                    DXGI_SWAP_CHAIN_DESC sd;
                    pSwapChain->GetDesc(&sd);
                    float displayWidth = static_cast<float>(sd.BufferDesc.Width);
                    float displayHeight = static_cast<float>(sd.BufferDesc.Height);
                    
                    // Use the fresh RTV for rendering
                    m_pLifecycleManager->RenderTick(m_hWindow, displayWidth, displayHeight, 
                                                  m_pContext, frameRenderTargetView);
                    
                    // Clean up the per-frame render target view
                    frameRenderTargetView->Release();
                } else {
                    LOG_WARN("[D3DRenderHook] Failed to create per-frame RTV");
                }
            }
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

        // Note: We no longer create a cached render target view here
        // Instead, we create a fresh RTV every frame in DetourPresent() to handle resize properly

        // Hook WndProc
        m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
        if (!m_pOriginalWndProc) {
            LOG_ERROR("[D3DRenderHook] Failed to hook WndProc");
            // Release partially initialized resources before cleanup
            if (m_pContext) {
                m_pContext->Release();
                m_pContext = nullptr;
            }
            if (m_pDevice) {
                m_pDevice->Release();
                m_pDevice = nullptr;
            }
            CleanupD3DResources(false);
            return false;
        }

        // Initialize ImGui
        if (!ImGuiManager::Initialize(m_pDevice, m_pContext, m_hWindow)) {
            LOG_ERROR("[D3DRenderHook] Failed to initialize ImGui");
            CleanupD3DResources(true);
            return false;
        }

        m_isInit = true;
        LOG_INFO("[D3DRenderHook] D3D resources and ImGui initialized successfully");
        return true;
    }

    LRESULT __stdcall D3DRenderHook::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        // Track mouse button states
        if (uMsg == WM_RBUTTONDOWN) m_rightMouseDown = true;
        else if (uMsg == WM_RBUTTONUP) m_rightMouseDown = false;
        else if (uMsg == WM_LBUTTONDOWN) m_leftMouseDown = true;
        else if (uMsg == WM_LBUTTONUP) m_leftMouseDown = false;

        // Only process ImGui input when initialized and window is open
        if (m_isInit && AppState::Get().IsVisionWindowOpen()) {
            // Check if cursor is hidden (camera rotation mode)
            CURSORINFO curInfo{};
            curInfo.cbSize = sizeof(CURSORINFO);
            GetCursorInfo(&curInfo);
            bool isCursorHidden = !(curInfo.flags & CURSOR_SHOWING);
            
            // For mouse button events, only call ImGui handler if we're over ImGui or an item is active
            bool isMouseButtonEvent = (uMsg >= WM_LBUTTONDOWN && uMsg <= WM_XBUTTONDBLCLK);
            bool shouldCallImGuiHandler = !isMouseButtonEvent || 
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ||
                ImGui::IsAnyItemActive();
            
            if (shouldCallImGuiHandler) {
                ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
            }
            
            // Get ImGui IO state AFTER calling handler (if we called it)
            ImGuiIO& io = ImGui::GetIO();
            
            // Update hover tracking for mouse move
            if (uMsg == WM_MOUSEMOVE) {
                m_wasOverImGuiWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
            }
            
            // For mouse button events, use selective blocking
            switch (uMsg) {
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDBLCLK:
                case WM_MBUTTONDBLCLK:
                case WM_XBUTTONDBLCLK:
                    // Only block if we called ImGui handler AND ImGui wants the mouse
                    if (shouldCallImGuiHandler && io.WantCaptureMouse) {
                        return 1;
                    }
                    else {
                        // CRITICAL: Clear any active ImGui items when clicking outside
                        ImGui::ClearActiveID();
                    }
                    break;
                    
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                    // Only block if an item is currently active (being dragged)
                    if (ImGui::IsAnyItemActive()) {
                        return 1;
                    }
                    break;
                    
                case WM_MOUSEWHEEL:
                    // Always call handler for mouse wheel, then check if ImGui wants it
                    if (!shouldCallImGuiHandler) {
                        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    }
                    if (io.WantCaptureMouse) {
                        return 1;
                    }
                    break;
                    
                case WM_MOUSEMOVE:
                    // Never block mouse move
                    break;
                    
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                    // Always call handler for keyboard events
                    if (!shouldCallImGuiHandler) {
                        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    }
                    if (io.WantCaptureKeyboard) {
                        return 1;
                    }
                    break;
                    
                case WM_CHAR:
                    // Always call handler for char events
                    if (!shouldCallImGuiHandler) {
                        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    }
                    if (io.WantTextInput) {
                        return 1;
                    }
                    break;
            }
        }

        // Pass to original game window procedure
        return m_pOriginalWndProc ? CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam)
            : DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

} // namespace kx::Hooking

#endif // !GW2AL_BUILD
