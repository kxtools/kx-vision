#pragma once

namespace kx {

    bool InitializeHooks();
    bool InitializeGameThreadHook();
    void CleanupHooks();

    namespace Hooking {
        void __fastcall DetourGameThread(void* pInst, int frame_time);
    }

} // namespace kx
