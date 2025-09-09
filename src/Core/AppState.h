#pragma once

#include <atomic>
#include <cstdint> // For uintptr_t

namespace kx {

    // --- User-configurable settings ---
    struct Settings {
        bool showVisionWindow = true;
        bool espEnabled = true;
        bool espRenderBox = true;
        bool espRenderDistance = true;
        bool espRenderDot = true;
    };

    extern Settings g_settings;


    // --- Status Information ---
    enum class HookStatus {
        Unknown,
        OK,
        Failed
    };

    extern HookStatus g_presentHookStatus;

    // --- App State ---
    extern bool g_isVisionWindowOpen; // Controls GUI window visibility and main loop unload trigger
    
    // --- Shutdown Synchronization ---
    extern std::atomic<bool> g_isShuttingDown; // Flag to signal shutdown to hooks

} // namespace kx
