#pragma once

#include <cstdint>

namespace kx {

    bool InitializeHooks();
    bool InitializeGameThreadHook();
    void CleanupHooks();

    namespace Hooking {
        uintptr_t __fastcall DetourGameThread(uintptr_t a1, uintptr_t a2);
    }

} // namespace kx
