#pragma once

#include <atomic>

namespace kx {

    // --- NEW: Category-specific settings ---
    struct PlayerEspSettings {
        bool enabled = true;
        bool renderBox = true;
        bool renderDistance = true;
        bool renderDot = true;
        bool renderDetails = false;
        // Future features from zenith can be added here:
        // SnaplineType snaplineType = SnaplineType::Disabled;
    };

    struct NpcEspSettings {
        bool enabled = true;
        bool renderBox = true;
        bool renderDistance = true;
        bool renderDot = true;
        bool renderDetails = false;
        // Add attitude filtering like in zenith
        int ignoredAttitude = 0; // Bitmask for GW2::Attitude
        // Add specific colors
        // ColorRGBA friendlyColor = { 0, 255, 100, 220 };
        // etc.
    };

    struct ObjectEspSettings {
        bool enabled = true;
        bool renderBox = true;
        bool renderDistance = true;
        bool renderDot = true;
        bool renderDetails = false;
        // Add gadget type filtering like in zenith
        int ignoredGadgets = 0; // Bitmask for gadget types
    };


    // --- User-configurable settings ---
    struct Settings {
        bool showVisionWindow = true;

        // Replace old flat settings with new structs
        PlayerEspSettings playerESP;
        NpcEspSettings npcESP;
        ObjectEspSettings objectESP;

        // Keep global settings
        bool espUseDistanceLimit = true;
        float espRenderDistanceLimit = 250.0f;
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
