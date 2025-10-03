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

    /**
     * @brief Distance-based rendering configuration
     * 
     * Controls how entities are culled and faded based on distance.
     * Two modes with different philosophies:
     * - Limit Mode: Natural Integration - mimics game's native behavior
     * - No Limit Mode: Maximum Information Clarity - adaptive to scene
     */
    struct DistanceSettings {
        // --- Distance Limiting ---
        bool useDistanceLimit = true;           // Enable/disable distance-based culling
        float renderDistanceLimit = 90.0f;      // Hard cutoff distance (mimics game's native culling range)
    };

    /**
     * @brief Scaling curve configuration
     * 
     * Controls how entity sizes shrink with distance using the formula:
     * scale = distanceFactor / (distanceFactor + distance^exponent)
     * 
     * The distanceFactor determines where 50% scale occurs.
     * The exponent controls the curve shape (higher = more aggressive at distance).
     */
    struct ScalingSettings {
        // --- Shared Settings (Both Modes) ---
        float scalingStartDistance = 20.0f;     // Distance before scaling begins (mimics game camera-to-player offset)
        float minScale = 0.1f;                  // Minimum scale multiplier (10% - allows extreme shrinking, protected by min sizes)
        float maxScale = 1.0f;                  // Maximum scale multiplier (100% - no magnification for natural feel)
        
        // --- Limit Mode (90m range) ---
        float limitDistanceFactor = 110.0f;     // 50% scale at 110m (just past render limit for meaningful scaling)
        float limitScalingExponent = 1.2f;      // Moderate curve - balanced shrinking over 0-90m range
        
        // --- No Limit Mode (Adaptive range) ---
        float noLimitScalingExponent = 1.2f;    // Balanced curve for long distances (distanceFactor auto-calculated from scene)
        // Note: distanceFactor = adaptiveFarPlane / 2 (automatic 50% scale at midpoint)
    };

    /**
     * @brief Base sizes for ESP elements before scaling
     * 
     * These are the "100% scale" sizes. Distance-based scaling multiplies these values.
     * Minimum size protections prevent elements from becoming unusably small.
     */
    struct ElementSizeSettings {
        // --- Text ---
        float baseFontSize = 20.0f;             // Large, bold font for readability
        float minFontSize = 9.0f;               // Absolute minimum font size (readability floor)
        
        // --- Shapes ---
        float baseDotRadius = 3.0f;             // Center dot size (matches font scale)
        float baseBoxThickness = 2.0f;          // Bounding box line thickness
        float baseBoxHeight = 100.0f;           // Player/NPC box height
        float baseBoxWidth = 60.0f;             // Player/NPC box width
        
        // --- Health Bars ---
        float baseHealthBarWidth = 65.0f;       // Health bar width (sized to match box)
        float baseHealthBarHeight = 8.0f;       // Health bar height (visible but not obtrusive)
    };

    struct Settings {
        // Category-specific ESP settings
        PlayerEspSettings playerESP;
        NpcEspSettings npcESP;
        ObjectEspSettings objectESP;

        // Organized configuration groups
        DistanceSettings distance;
        ScalingSettings scaling;
        ElementSizeSettings sizes;
        
        // Performance settings
        float espUpdateRate = 60.0f;            // ESP updates per second (60 = smooth, lower = better performance)
        
        // Enhanced filtering options
        bool hideDepletedNodes = true;          // Hide depleted resource nodes (visual clutter reduction)
        
        // Debug options
#ifdef _DEBUG
        bool enableDebugLogging = true;         // Enable verbose logging by default in debug builds
        bool showDebugAddresses = true;         // Show entity memory addresses on ESP
#else
        bool enableDebugLogging = false;        // Disable verbose logging by default in release builds
        bool showDebugAddresses = false;
#endif
    };

} // namespace kx