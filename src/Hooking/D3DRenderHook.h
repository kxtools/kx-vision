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

    // Forward declare ImGuiManager to avoid include dependency cycle if needed
    // class ImGuiManager;

    // Define the Present function pointer type
    typedef long(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

    /**
     * @brief Manages the D3D11 Present hook, WndProc hooking, and ImGui integration.
     */
    class D3DRenderHook {
    public:
        /**
         * @brief Initializes the Present hook, finds the Present function,
         *        hooks WndProc, and prepares for ImGui rendering.
         * @return True if successful, false otherwise.
         */
        static bool Initialize();

        /**
         * @brief Initialize from an existing D3D device (GW2AL mode)
         * @param device The D3D11 device provided by GW2AL
         * @param swapChain The swap chain provided by GW2AL
         * @return True if successful, false otherwise.
         */
        static bool InitializeFromDevice(ID3D11Device* device, IDXGISwapChain* swapChain);

        /**
         * @brief Handle swap chain resize events
         * @param pSwapChain The swap chain being resized
         */
        static void OnResize(IDXGISwapChain* pSwapChain);

        /**
         * @brief Cleans up resources, restores the original WndProc, and
         *        requests removal of the Present hook via HookManager.
         */
        static void Shutdown();

        /**
         * @brief Checks if the hook and ImGui integration have been initialized.
         * @return True if initialized, false otherwise.
         */
        static bool IsInitialized();

        /**
         * @brief Sets the AppLifecycleManager for accessing Camera and MumbleLink data
         * @param lifecycleManager Pointer to the AppLifecycleManager instance
         */
        static void SetLifecycleManager(AppLifecycleManager* lifecycleManager);

        static ID3D11Device* GetDevice() { return m_pDevice; }
        static ID3D11DeviceContext* GetContext() { return m_pContext; }
        static ID3D11RenderTargetView* GetMainRenderTargetView() { return m_pMainRenderTargetView; }

        static HWND GetWindowHandle() { return m_hWindow; }

    private:
        // Prevent instantiation
        D3DRenderHook() = delete;
        ~D3DRenderHook() = delete;

        // --- Hook Target & Original ---
        static Present m_pTargetPresent;   // Address of the original Present function
        static Present m_pOriginalPresent; // Trampoline to the original Present function

        // --- D3D Resources ---
        static bool m_isInit;                   // Initialization flag
        static HWND m_hWindow;                  // Handle to the game window
        static ID3D11Device* m_pDevice;         // D3D11 Device
        static ID3D11DeviceContext* m_pContext; // D3D11 Device Context
        static ID3D11RenderTargetView* m_pMainRenderTargetView; // Main render target

        // --- WndProc Hooking ---
        static WNDPROC m_pOriginalWndProc; // Pointer to the game's original WndProc

        // --- Lifecycle Manager ---
        static AppLifecycleManager* m_pLifecycleManager; // Pointer to AppLifecycleManager for game state

        // --- Private Methods ---
        /**
         * @brief Finds the address of the IDXGISwapChain::Present function.
         * @return True if the pointer was found, false otherwise.
         */
        static bool FindPresentPointer();

        /**
         * @brief The detour function for IDXGISwapChain::Present.
         */
        static HRESULT __stdcall DetourPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

        /**
         * @brief The replacement Window Procedure (WndProc).
         */
        static LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // --- Helper Methods for DetourPresent ---
        
        /**
         * @brief One-time initialization of D3D resources, ImGui, and WndProc hook
         * @param pSwapChain The swap chain to initialize from
         * @return True if initialization successful, false otherwise
         */
        static bool InitializeD3DResources(IDXGISwapChain* pSwapChain);

        /**
         * @brief Render the ImGui frame
         */
        static void RenderFrame();
    };

} // namespace kx::Hooking