#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <d3d11.h>
#include <windows.h>
#include "HookManager.h"
#pragma comment(lib, "d3d11.lib")

namespace kx {
    class AppLifecycleManager;
}

namespace kx::Hooking {

    // DXGI VTable indices
    namespace VTableIndices {
        constexpr size_t DXGI_PRESENT = 8;  // IDXGISwapChain::Present function index
    }

    typedef long(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

    /**
     * @brief Manages the D3D11 Present hook, WndProc hooking, and ImGui integration.
     */
    class D3DRenderHook {
    public:
        static bool Initialize();
        static bool InitializeFromDevice(ID3D11Device* device, IDXGISwapChain* swapChain);
        static void OnResize(IDXGISwapChain* pSwapChain);
        static void Shutdown();
        static bool IsInitialized();
        static void SetLifecycleManager(AppLifecycleManager* lifecycleManager);

        static ID3D11Device* GetDevice() { return m_pDevice; }
        static ID3D11DeviceContext* GetContext() { return m_pContext; }

        static HWND GetWindowHandle() { return m_hWindow; }

        static bool InstallWndProc(HWND hwnd);
        static void UninstallWndProc();
        static void SyncOverlayInputState();

        // Render-thread ImGui input (WndProc enqueues; Present drains/applies).
        static void DrainOverlayInputQueue();      // Before ImGui_Impl* / NewFrame
        static void ApplyDeferredOverlayInput();   // After NewFrame (focus-loss input clear)
        static void CaptureOverlayRouting();       // After UI build (publish WantCapture* atomics)

    private:
        D3DRenderHook() = delete;
        ~D3DRenderHook() = delete;

        // Hook state
        static Present m_pTargetPresent;
        static Present m_pOriginalPresent;

        // D3D resources
        static bool m_isInit;
        static bool m_presentHookInstalled;
        static HookHandle m_presentHook;
        static HWND m_hWindow;
        static ID3D11Device* m_pDevice;
        static ID3D11DeviceContext* m_pContext;
        static AppLifecycleManager* m_pLifecycleManager;

        // Methods
        static bool FindPresentPointer();
        static HRESULT __stdcall DetourPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
        static bool InitializeD3DResources(IDXGISwapChain* pSwapChain);
        static void CleanupD3DResources(bool includeWndProc = true);
    };

} // namespace kx::Hooking
