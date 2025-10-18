#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <d3d11.h>
#include <windows.h>
#pragma comment(lib, "d3d11.lib")

namespace kx {
    class AppLifecycleManager;
}

namespace kx::Hooking {

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
        static ID3D11RenderTargetView* GetMainRenderTargetView() { return m_pMainRenderTargetView; }

        static HWND GetWindowHandle() { return m_hWindow; }

    private:
        D3DRenderHook() = delete;
        ~D3DRenderHook() = delete;

        // Hook state
        static Present m_pTargetPresent;
        static Present m_pOriginalPresent;

        // D3D resources
        static bool m_isInit;
        static HWND m_hWindow;
        static ID3D11Device* m_pDevice;
        static ID3D11DeviceContext* m_pContext;
        static ID3D11RenderTargetView* m_pMainRenderTargetView;
        static WNDPROC m_pOriginalWndProc;
        static AppLifecycleManager* m_pLifecycleManager;

        // WndProc state (DLL mode only)
        static bool m_rightMouseDown;
        static bool m_leftMouseDown;
        static bool m_wasOverImGuiWindow;

        // Methods
        static bool FindPresentPointer();
        static HRESULT __stdcall DetourPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
        static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static bool InitializeD3DResources(IDXGISwapChain* pSwapChain);
        static void RenderFrame();
        static void CleanupD3DResources(bool includeWndProc = true);
    };

} // namespace kx::Hooking