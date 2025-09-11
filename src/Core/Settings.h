#pragma once

namespace kx {

    // --- Category-specific settings ---
    struct PlayerEspSettings {
        bool enabled = true;
        bool renderBox = true;
        bool renderDistance = true;
        bool renderDot = true;
        bool renderDetails = false;
        bool renderHealthBar = true;
        bool showProfession = true;
        bool showRace = true;
        bool showArmorWeight = true;
        bool showLocalPlayer = false; // Hide local player by default
        // Future features from zenith can be added here:
        // SnaplineType snaplineType = SnaplineType::Disabled;
    };

    struct NpcEspSettings {
        bool enabled = true;
        bool renderBox = true;
        bool renderDistance = true;
        bool renderDot = true;
        bool renderDetails = false;
        bool renderHealthBar = true;
        // Enhanced attitude filtering using the new enum
        bool showFriendly = true;
        bool showHostile = true;
        bool showNeutral = true;
        bool showIndifferent = true;
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
        
        // Performance settings
        float espUpdateRate = 60.0f; // ESP updates per second (lower = better performance)
        
        // New enhanced filtering options
        bool enableSmartFiltering = true; // Use the enhanced enum-based filtering
        bool hideDepletedNodes = true;    // Hide depleted resource nodes
        bool prioritizeImportant = true;  // Give priority to important objects
        
        // Debug options
        bool enableDebugLogging = false;  // Enable detailed debug logging
    };

} // namespace kx