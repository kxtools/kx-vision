#pragma once

#include <atomic>
#include <cstdint> // For uintptr_t

namespace kx {

    // --- Status Information ---
    enum class HookStatus {
        Unknown,
        OK,
        Failed
    };

    extern HookStatus g_presentHookStatus;

    // --- UI State ---
    extern bool g_isVisionWindowOpen; // Controls GUI window visibility and main loop unload trigger
    extern bool g_showVisionWindow;   // Controls GUI visibility (toggle via hotkey)
    extern bool g_espEnabled;           // Controls ESP rendering (true by default)
    extern bool g_espRenderBox;         // Controls rendering of the ESP box
    extern bool g_espRenderDistance;    // Controls rendering of the ESP distance text
    extern bool g_espRenderDot;         // Controls rendering of the ESP mini dot

    // --- Shutdown Synchronization ---
    extern std::atomic<bool> g_isShuttingDown; // Flag to signal shutdown to hooks

} // namespace kx