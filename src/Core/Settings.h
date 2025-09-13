#pragma once

namespace kx {

    // --- Category-specific settings ---
    struct PlayerEspSettings {
        bool enabled = true;
        bool renderBox = false;
        bool renderDistance = false;
        bool renderDot = false;
        bool renderDetails = false;
        bool renderHealthBar = true;
        bool renderPlayerName = true;  // Show player names by default for natural identification
        bool showProfession = true;
        bool showRace = true;
        bool showArmorWeight = true;
        bool showLocalPlayer = false; // Hide local player by default
        // Future features from zenith can be added here:
        // SnaplineType snaplineType = SnaplineType::Disabled;
    };

    struct NpcEspSettings {
        bool enabled = true;
        bool renderBox = false;
        bool renderDistance = false;
        bool renderDot = false;
        bool renderDetails = false;
        bool renderHealthBar = true;
        // Enhanced attitude filtering using the new enum
        bool showFriendly = true;
        bool showHostile = true;
        bool showNeutral = true;
        bool showIndifferent = true;
        // Health-based filtering
        bool showDeadNpcs = false;  // Show NPCs with 0 HP (dead/defeated enemies)
        // Add specific colors
        // ColorRGBA friendlyColor = { 0, 255, 100, 220 };
        // etc.
    };

    struct ObjectEspSettings {
        bool enabled = true;
        bool renderBox = false;
        bool renderDistance = false;
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
    };

    // --- User-configurable settings ---
    namespace AppConfig {
#ifdef _DEBUG
        constexpr int DEFAULT_LOG_LEVEL = 1; // INFO
#else
        constexpr int DEFAULT_LOG_LEVEL = 3; // ERR
#endif
    }

    struct Settings {
#ifdef _DEBUG
        bool showVisionWindow = true;   // Show menu by default in debug builds
#else
        bool showVisionWindow = false;  // Hide menu by default in release builds
#endif

        // Replace old flat settings with new structs
        PlayerEspSettings playerESP;
        NpcEspSettings npcESP;
        ObjectEspSettings objectESP;

        // Keep global settings
        bool espUseDistanceLimit = true;
        float espRenderDistanceLimit = 90.0f;  // For more natural look
        
        // ESP Scaling Configuration
        float espMinScale = 0.3f;      // Minimum scale factor for distant entities
        float espMaxScale = 2.0f;      // Maximum scale factor for close entities  
        float espScaleFactor = 35.0f;  // Base scaling divisor (calibrated for typical distances)
        
        // Performance settings
        float espUpdateRate = 60.0f; // ESP updates per second (lower = better performance)
        
        // Enhanced filtering options
        bool hideDepletedNodes = true;    // Hide depleted resource nodes
        
        // Debug options
#ifdef _DEBUG
        bool enableDebugLogging = true;   // Enable verbose logging by default in debug builds
#else
        bool enableDebugLogging = false;  // Disable verbose logging by default in release builds
#endif
    };

} // namespace kx