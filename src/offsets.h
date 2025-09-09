#pragma once
#include <cstdint>

namespace GW2LIB_OFFSETS {
    // WvContext Offsets
    constexpr uintptr_t wvctxStatus = 0x58;
    constexpr uintptr_t wvctxPtrToRenderer = 0x78;

    // WvGameRenderer Offsets
    constexpr uintptr_t renderer_ViewMatrix = 0x10;
    constexpr uintptr_t renderer_ProjMatrix = 0x54;
    constexpr uintptr_t renderer_CameraPosition = 0x80;
}
