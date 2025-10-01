#pragma once

namespace kx {

    /**
     * @brief Orchestrates the initialization of essential hooks (D3D Present, WndProc).
     * @return True if essential hooks were initialized, false otherwise.
     */
    bool InitializeHooks();

    /**
     * @brief Initializes the game thread hook after AddressManager is ready.
     * @return True if game thread hook was created successfully, false otherwise.
     */
    bool InitializeGameThreadHook();

    /**
     * @brief Orchestrates the shutdown and cleanup of all hooks and related systems.
     */
    void CleanupHooks();

    namespace Hooking {
        void __fastcall DetourGameThread(void* pInst, int frame_time);
    }

} // namespace kx
