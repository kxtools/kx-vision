#pragma once

namespace kx {

    // --- ESP Display Modes ---
    
    /**
     * @brief Display mode for player gear/equipment stats
     */
    enum class GearDisplayMode {
        Off = 0,        // No gear information displayed
        Compact = 1,    // Compact view: Show stat names with counts
        Attributes = 2, // Show top 3 dominant attributes with percentages
        Detailed = 3    // Detailed view: Full list of all gear slots and stats
    };

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
        GearDisplayMode gearDisplayMode = GearDisplayMode::Off;
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
        
        // Gadget Type Filters
        bool showResourceNodes = true;    // Type 19
        bool showWaypoints = true;        // Type 18
        bool showVistas = true;           // Type 24
        bool showCraftingStations = true; // Type 5
        bool showAttackTargets = true;    // Type 16
        bool showPlayerCreated = true;    // Type 23
        bool showInteractables = true;    // Type 12 (covers chests, etc.)
        bool showDoors = true;            // Type 6
        bool showPortals = true;          // Type 17 (MapPortal)
        bool showDestructible = true;     // Type 1
        bool showPoints = true;           // Type 2 (PvP points)
        bool showPlayerSpecific = true;   // Type 14
        bool showProps = true;            // Type 20
        bool showBuildSites = true;       // Type 25
        bool showBountyBoards = true;     // Type 11
        bool showRifts = true;            // Type 13
        bool showGeneric = false;         // Type 3
        bool showUnknown = true;          // For any type not explicitly handled
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
        // Replace old flat settings with new structs
        PlayerEspSettings playerESP;
        NpcEspSettings npcESP;
        ObjectEspSettings objectESP;

        // Keep global settings
        bool espUseDistanceLimit = true;
        float espRenderDistanceLimit = 90.0f;  // For more natural look

        // ESP Scaling Configuration
        float espMinScale = 0.1f;
        float espMaxScale = 1.0f;
        float espScalingStartDistance = 20.0f; // The recommended value.
        float espDistanceFactor = 110.0f;      // For LIMIT MODE - The balanced, natural curve
        float espScalingExponent = 1.2f;       // For LIMIT MODE - A slightly accelerating curve feels most natural.
        
        // --- Scaling for "No Limit" Mode ---
        float noLimitScalingExponent = 1.2f;   // Controls curve shape in No Limit mode (distanceFactor is dynamic)

        // Base size settings for scalable elements
        float espMinFontSize = 9.0f;
        float espBaseFontSize = 20.0f;         // large, bold font.
        float espBaseDotRadius = 3.0f;         // Scaled up slightly to match the larger font.
        float espBaseHealthBarWidth = 65.0f;   // Sized to look good with the 60px box width.
        float espBaseHealthBarHeight = 8.0f;   // Made slightly thicker to match the new width.
        float espBaseBoxThickness = 2.0f;      // Increased thickness
        float espBaseBoxHeight = 100.0f;
        float espBaseBoxWidth = 60.0f;
        
        // Performance settings
        float espUpdateRate = 60.0f; // ESP updates per second (lower = better performance)
        
        // Enhanced filtering options
        bool hideDepletedNodes = true;    // Hide depleted resource nodes
        
        // Debug options
#ifdef _DEBUG
        bool enableDebugLogging = true;   // Enable verbose logging by default in debug builds
        bool showDebugAddresses = true; // Show entity memory addresses on ESP
#else
        bool enableDebugLogging = false;  // Disable verbose logging by default in release builds
        bool showDebugAddresses = false;
#endif
    };

} // namespace kx