#pragma once

#include <atomic>
#include "../Game/GameEnums.h"

namespace kx {

    // --- NEW: Category-specific settings ---
    struct PlayerEspSettings {
        bool enabled = true;
        bool renderBox = true;
        bool renderDistance = true;
        bool renderDot = true;
        bool renderDetails = false;
        bool showProfession = true;
        bool showRace = true;
        bool showArmorWeight = true;
        // Future features from zenith can be added here:
        // SnaplineType snaplineType = SnaplineType::Disabled;
    };

    struct NpcEspSettings {
        bool enabled = true;
        bool renderBox = true;
        bool renderDistance = true;
        bool renderDot = true;
        bool renderDetails = false;
        // Enhanced attitude filtering using the new enum
        bool showFriendly = true;
        bool showHostile = true;
        bool showNeutral = true;
        bool showIndifferent = true;
        int ignoredAttitude = 0; // Bitmask for NPC attitude types (legacy support)
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
        
        // Enhanced gadget filtering using the new enum
        bool showResourceNodes = true;
        bool showWaypoints = true;
        bool showVistas = true;
        bool showCraftingStations = true;
        bool showAttackTargets = true;
        bool showPlayerCreated = true;
        bool showInteractables = true;
        bool showDoors = false;
        bool showPortals = true;
        bool onlyImportantGadgets = false; // Only show important gadget types
        
        int ignoredGadgets = 0; // Bitmask for gadget types (legacy support)
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
        
        // New enhanced filtering options
        bool enableSmartFiltering = true; // Use the enhanced enum-based filtering
        bool hideDepletedNodes = true;    // Hide depleted resource nodes
        bool prioritizeImportant = true;  // Give priority to important objects
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
