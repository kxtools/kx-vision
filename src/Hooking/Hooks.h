#pragma once

namespace kx {

    /**
     * @brief Orchestrates the initialization of all required hooks.
     * @return True if essential hooks (like Present) were initialized, false otherwise.
     */
    bool InitializeHooks();

    /**
     * @brief Orchestrates the shutdown and cleanup of all hooks and related systems.
     */
    void CleanupHooks();

    namespace Hooking {
        void __fastcall DetourGameThread(void* pInst, int frame_time);
    }

} // namespace kx
