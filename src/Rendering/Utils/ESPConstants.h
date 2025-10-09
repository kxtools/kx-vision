#pragma once

#include "../../../libs/ImGui/imgui.h"

namespace kx {

    namespace CombatEffects {
        // --- Adaptive Damage Accumulator (Pixel-Based Tuning) ---
	    constexpr float    DESIRED_CHUNK_PIXELS = 20.0f; // The target on-screen width for a satisfying chunk.
	    constexpr uint64_t MAX_FLUSH_INTERVAL_MS = 1200; // A slightly longer responsive fallback.

        // --- Core Combat Feedback (TUNED FOR PUNCHY HITS) ---
        // A 200ms hold followed by a 400ms fade provides satisfying impact on every hit.
        constexpr uint64_t DAMAGE_FLASH_HOLD_DURATION_MS = 200;
        constexpr uint64_t DAMAGE_FLASH_FADE_DURATION_MS = 400;
        constexpr uint64_t DAMAGE_FLASH_TOTAL_DURATION_MS = DAMAGE_FLASH_HOLD_DURATION_MS + DAMAGE_FLASH_FADE_DURATION_MS;

        // --- Healing Feedback (TUNED FOR CLARITY) ---
        // A quick flash confirms the heal, and a 2s overlay shows the amount restored.
        constexpr uint64_t HEAL_FLASH_DURATION_MS = 150;
        constexpr uint64_t HEAL_OVERLAY_DURATION_MS = 2000;
        constexpr uint64_t HEAL_OVERLAY_FADE_DURATION_MS = 400;
        constexpr uint64_t BURST_HEAL_WINDOW_MS = 350; // Groups rapid heals

        // --- Death Animation (TUNED FOR A SATISFYING FINISH) ---
        // A 2.5s animation that gives a definitive and polished end-of-combat signal.
        constexpr uint64_t DEATH_BURST_DURATION_MS = 400;
        constexpr uint64_t DEATH_FINAL_FADE_DURATION_MS = 2100;
        constexpr uint64_t DEATH_ANIMATION_TOTAL_DURATION_MS = DEATH_BURST_DURATION_MS + DEATH_FINAL_FADE_DURATION_MS;

        // --- State Management ---
        constexpr uint64_t STATE_CLEANUP_THRESHOLD_MS = 3000;
    }

/**
 * @brief Minimum size constraints for entity visibility
 * 
 * These constants ensure that entities remain visible even at extreme distances
 * by enforcing minimum pixel dimensions for each entity type. Values are optimized
 * based on base sizes (90h×45w) and scaling system (minScale=0.1, ~22% for minimums).
 */
namespace MinimumSizes {
    // Player minimum sizes (maintains 2:1 humanoid aspect ratio)
    constexpr float PLAYER_MIN_HEIGHT = 20.0f;   // ~22% of base 90px height
    constexpr float PLAYER_MIN_WIDTH = 10.0f;    // Proportional: 20÷2.0 (maintains 2:1 ratio)
    
    // NPC minimum sizes (square boxes, larger for better visibility)
    constexpr float NPC_MIN_HEIGHT = 15.0f;      // ~33% of base 45px width (square rendering)
    constexpr float NPC_MIN_WIDTH = 15.0f;       // Square maintains recognizable shape
    
    // Gadget minimum sizes (circles, very small but distinct points)
    constexpr float GADGET_MIN_HEIGHT = 3.0f;    // ~44% of full-scale radius (base 6.75px = 45w × 0.15)
    constexpr float GADGET_MIN_WIDTH = 3.0f;     // Minimum for circle visibility at distance
}

/**
 * @brief Entity size ratios and proportions
 * 
 * Defines the relative sizing and proportions for different entity types.
 * These ratios determine how entity sizes relate to the base box dimensions.
 */
namespace EntitySizeRatios {
    // Gadget circle sizing (relative to base box width)
    constexpr float GADGET_CIRCLE_RADIUS_RATIO = 0.15f;  // Circle radius = baseBoxWidth × 0.15 (6.75px from 45px)
    // Results in 13.5px diameter (30% of player width) - small enough for "objects" not "characters"
    
    // NPC box sizing (uses base box width directly - see CalculateEntityBoxDimensions)
    // NPCs are 45×45 squares (1.0× player width) for visual consistency
}

/**
 * @brief Coordinate transformation constants
 * 
 * Constants used for converting between game world coordinates and MumbleLink coordinates.
 * The game uses different coordinate systems internally vs what's exposed through MumbleLink.
 */
namespace CoordinateTransform {
    // Scale factor for converting game world coordinates to MumbleLink meter-based units
    constexpr float GAME_TO_MUMBLE_SCALE_FACTOR = 1.23f;
}

/**
 * @brief Data extraction capacity constants
 * 
 * Initial capacity reservations for entity collections to minimize dynamic allocations
 * during frame data extraction. These values are based on actual game data analysis:
 * - PlayerList typically has ~134 players, we extract ~12 valid players
 * - CharacterList capacity ~9728, we extract ~29 NPCs 
 * - GadgetList capacity ~9216, we extract ~457 gadgets
 * - Values include safe buffer for peak scenarios and different map types
 */
namespace ExtractionCapacity {
    constexpr size_t PLAYERS_RESERVE = 64;     // ~12 typical + buffer for busy instances
    constexpr size_t NPCS_RESERVE = 128;       // ~29 typical + buffer for NPC-heavy areas  
    constexpr size_t GADGETS_RESERVE = 1024;   // ~457 typical + buffer for resource-rich zones
}

/**
 * @brief Adaptive scaling system constants
 * 
 * Constants used for the intelligent adaptive far plane and distance-based scaling system.
 * These values control how the system adapts to different map sizes and entity distributions.
 */
namespace AdaptiveScaling {
    // Adaptive far plane bounds (applies to gadgets/objects only)
    constexpr float FAR_PLANE_MIN = 100.0f;   // Minimum far plane for small instances/dungeons
    constexpr float FAR_PLANE_MAX = 3000.0f;  // Maximum far plane to prevent extreme outliers
    constexpr float FAR_PLANE_DEFAULT = 800.0f; // Fallback when no gadgets present (mid-range)
    
    // Minimum sample size for percentile calculation
    constexpr size_t MIN_ENTITIES_FOR_PERCENTILE = 10; // Need at least 10 entities for meaningful statistics
    
    // Distance factors for scaling calculation (50% scale point)
    constexpr float PLAYER_NPC_DISTANCE_FACTOR = 150.0f;  // Fixed factor for players/NPCs (they're limited to ~200m)
    constexpr float GADGET_MIN_DISTANCE_FACTOR = 150.0f;  // Minimum factor for gadgets (matches player/NPC baseline)
    
    // Alpha fading constants (Gadgets - adaptive long-range fade)
    constexpr float FADE_START_DISTANCE = 90.0f; // Start fading beyond game's natural entity culling range
    constexpr float MIN_ALPHA = 0.5f;            // Minimum opacity for gadgets at extreme distances (50%)
    
    // Alpha fading constants (Players/NPCs - subtle fixed-range fade)
    constexpr float PLAYER_NPC_FADE_START = 80.0f;  // Start fade at same point as Limit Mode (consistent feel)
    constexpr float PLAYER_NPC_FADE_END = 120.0f;   // End fade at earliest culling distance (game culls at 120-200m)
}

/**
 * @brief Scaling limits for ESP elements
 * 
 * Maximum and minimum bounds for scaled ESP elements to prevent extreme sizes
 * at very close or very distant ranges. These limits ensure visual quality and
 * prevent performance issues from oversized elements.
 */
namespace ScalingLimits {
    // Font size limits (base: 16px)
    constexpr float MAX_FONT_SIZE = 40.0f;           // Maximum font size (2.5x base, prevents giant text)
    // Min font size defined in Settings (9px) for user configurability
    
    // Box thickness limits (base: 2px)
    constexpr float MIN_BOX_THICKNESS = 1.0f;        // Minimum line thickness (prevents invisible lines)
    constexpr float MAX_BOX_THICKNESS = 10.0f;       // Maximum line thickness (5x base, prevents chunky borders)
    
    // Dot radius limits (base: 3px)
    constexpr float MIN_DOT_RADIUS = 1.0f;           // Minimum dot size (visible pixel)
    constexpr float MAX_DOT_RADIUS = 15.0f;          // Maximum dot size (5x base, prevents giant dots)
    
    // Health bar limits (base: 60w × 7h)
    constexpr float MIN_HEALTH_BAR_WIDTH = 10.0f;    // Minimum readable width
    constexpr float MAX_HEALTH_BAR_WIDTH = 200.0f;   // Maximum width (3.33x base, allows for rank multipliers)
    constexpr float MIN_HEALTH_BAR_HEIGHT = 2.0f;    // Minimum visible height
    constexpr float MAX_HEALTH_BAR_HEIGHT = 25.0f;   // Maximum height (3.57x base, allows for rank multipliers)
}

/**
 * @brief Rendering effect constants
 * 
 * Constants that control visual effects and rendering behaviors in the ESP system.
 * These values tune the visual experience and performance characteristics.
 */
namespace RenderingEffects {
    // Distance fade zone - entities fade out in the last 11% of their distance limit
    // e.g., with 90m limit: fade starts at 80m (90 * 0.89), ends at 90m
    constexpr float FADE_ZONE_PERCENTAGE = 0.11f;
    
    // Hostile player visibility enhancement (PvP combat awareness)
    // This multiplier is applied to ALL hostile player visual elements:
    // - Box border thickness
    // - Health bar width/height
    // - Player name font size
    constexpr float HOSTILE_PLAYER_VISUAL_MULTIPLIER = 2.0f;
}

/**
 * @brief ESP color constants for different entity types and attitudes
 * 
 * Provides a consistent color scheme across the ESP system:
 * - Players: Bright cyan/blue for easy team identification
 * - NPCs: Attitude-based colors following GW2 conventions
 * - Gadgets: Warm orange for interactable objects
 * 
 * Note: IM_COL32 takes colors in (R, G, B, A) order - Red, Green, Blue, Alpha
 */
namespace ESPColors {
    // Default text color (used across all detail rendering)
    constexpr unsigned int DEFAULT_TEXT = IM_COL32(255, 255, 255, 255);  // White

    // Player colors
    constexpr unsigned int PLAYER = IM_COL32(30, 144, 255, 230);  // Bright cyan/blue (dodger blue)
    
    // NPC colors based on attitude - unified palette
    constexpr unsigned int NPC_HOSTILE = IM_COL32(220, 50, 40, 210);      // Thematic, high-contrast crimson
    constexpr unsigned int NPC_FRIENDLY = IM_COL32(100, 255, 100, 210);   // Bright classic green
    constexpr unsigned int NPC_NEUTRAL = IM_COL32(127, 255, 0, 210);      // Electric chartreuse (yellow-green)
    constexpr unsigned int NPC_INDIFFERENT = IM_COL32(240, 240, 240, 210); // Clean bright white
    constexpr unsigned int NPC_UNKNOWN = IM_COL32(255, 0, 255, 210);      // Magenta - debug/unknown
    
    // Gadget colors
    constexpr unsigned int GADGET = IM_COL32(255, 165, 80, 200);  // Warm orange/amber

    // Bar colors
    constexpr unsigned int ENERGY_BAR = IM_COL32(0, 120, 255, 220); // Bright blue for energy
    
    // Summary/gear display colors
    constexpr unsigned int SUMMARY_TEXT_RGB = IM_COL32(200, 210, 255, 255);  // Light periwinkle (for gear stats/summaries)
    // Note: Use with custom alpha - RGB(200, 210, 255) is a soft, readable blue-tinted white
}

/**
 * @brief Colors for different item rarities, following in-game conventions.
 *
 * Provides a standardized color palette for item rarities to be used in gear
 * displays and other UI elements. Colors are optimized for readability.
 *
 * Note: IM_COL32 takes colors in (R, G, B, A) order.
 */
namespace RarityColors {
    constexpr unsigned int JUNK = IM_COL32(170, 170, 170, 255);       // Gray (#AAAAAA)
    constexpr unsigned int COMMON = IM_COL32(255, 255, 255, 255);     // White (readability over authenticity)
    constexpr unsigned int FINE = IM_COL32(98, 164, 218, 255);        // Blue (#62A4DA)
    constexpr unsigned int MASTERWORK = IM_COL32(26, 147, 6, 255);      // Green (#1a9306)
    constexpr unsigned int RARE = IM_COL32(252, 208, 11, 255);        // Yellow (#fcd00b)
    constexpr unsigned int EXOTIC = IM_COL32(255, 164, 5, 255);       // Orange (#ffa405)
    constexpr unsigned int ASCENDED = IM_COL32(251, 62, 141, 255);    // Pink (#fb3e8d)
    constexpr unsigned int LEGENDARY = IM_COL32(139, 79, 219, 255);   // Bright Purple (#8B4FDB)
    constexpr unsigned int DEFAULT = COMMON;                         // Default to white
}

/**
 * @brief Screen space culling constants
 * 
 * Constants used for determining when entities are visible on screen.
 */
namespace ScreenCulling {
    // Margin for screen bounds culling - allows partially visible entities to render
    constexpr float VISIBILITY_MARGIN = 50.0f;
}

/**
 * @brief ESP layout constants for positioning and sizing visual elements.
 *
 * Optimized values based on base sizes (16px font, 45w×90h boxes, 60w×7h HP bars)
 * and industry-standard UI spacing. All alpha values use 0-255 range.
 */
namespace RenderingLayout {
    // Common (base rendering elements)
    constexpr float TEXT_SHADOW_OFFSET = 1.0f;       // 1px shadow for crisp text readability
    constexpr float DOT_RADIUS_MULTIPLIER = 0.8f;    // 80% of base radius (2.4px from 3px base)

    // Standalone Health Bar (horizontal bars below entities)
    constexpr float STANDALONE_HEALTH_BAR_Y_OFFSET = 12.0f;          // 12px below entity (compact spacing)
    constexpr float STANDALONE_HEALTH_BAR_BG_ROUNDING = 1.0f;        // Subtle 1px rounding
    constexpr float STANDALONE_HEALTH_BAR_BORDER_ROUNDING = 1.0f;    // Matches background
    constexpr float STANDALONE_HEALTH_BAR_BORDER_THICKNESS = 2.0f;   // crisp border
    constexpr float STANDALONE_HEALTH_BAR_BG_ALPHA = 180.0f;         // ~71% opacity (readable)
    constexpr float STANDALONE_HEALTH_BAR_HEALTH_ALPHA = 220.0f;     // ~86% opacity (prominent)
    constexpr float STANDALONE_HEALTH_BAR_BORDER_ALPHA = 100.0f;     // ~39% opacity (subtle)

    // Player Name (above entity, prominent label)
    constexpr float PLAYER_NAME_Y_OFFSET = 22.0f;             // 22px above entity (reduced from 25px)
    constexpr float PLAYER_NAME_BG_PADDING_X = 4.0f;          // 4px horizontal padding
    constexpr float PLAYER_NAME_BG_PADDING_Y = 2.0f;          // 2px vertical padding
    constexpr float PLAYER_NAME_BG_ROUNDING = 3.0f;           // 3px rounded corners
    constexpr float PLAYER_NAME_BORDER_THICKNESS = 1.0f;      // 1px crisp border
    constexpr float PLAYER_NAME_BG_ALPHA = 60.0f;             // ~24% opacity (very subtle)
    constexpr float PLAYER_NAME_BORDER_ALPHA = 120.0f;        // ~47% opacity (visible outline)
    constexpr float PLAYER_NAME_SHADOW_ALPHA = 180.0f;        // ~71% opacity (strong shadow)
    constexpr float PLAYER_NAME_TEXT_ALPHA = 220.0f;          // ~86% opacity (clear text)

    // Bounding Box (corner-style box rendering)
    constexpr float BOX_CORNER_SIZE_MULTIPLIER = 4.0f;        // Corner length = thickness × 4 (8px at 2px thick)

    // Distance Text (compact metric label)
    constexpr float DISTANCE_TEXT_Y_OFFSET = 15.0f;           // 15px gap from anchor (clean separation from box)
    constexpr float DISTANCE_TEXT_BG_PADDING_X = 3.0f;        // 3px horizontal padding (increased from 2px)
    constexpr float DISTANCE_TEXT_BG_PADDING_Y = 1.0f;        // 1px vertical padding (compact)
    constexpr float DISTANCE_TEXT_BG_ROUNDING = 2.0f;         // 2px rounded corners
    constexpr float DISTANCE_TEXT_BG_ALPHA = 60.0f;           // ~24% opacity (very subtle)
    constexpr float DISTANCE_TEXT_SHADOW_ALPHA = 180.0f;      // ~71% opacity (strong shadow)
    constexpr float DISTANCE_TEXT_TEXT_ALPHA = 220.0f;        // ~86% opacity (clear text)

    // Details Text (multi-line entity information)
    constexpr float DETAILS_TEXT_Y_OFFSET = 5.0f;             // 5px gap from anchor
    constexpr float DETAILS_TEXT_BG_PADDING_X = 4.0f;         // 4px horizontal padding (increased from 3px)
    constexpr float DETAILS_TEXT_BG_PADDING_Y = 2.0f;         // 2px vertical padding (increased from 1px)
    constexpr float DETAILS_TEXT_BG_ROUNDING = 2.0f;          // 2px rounded corners (increased from 1px)
    constexpr float DETAILS_TEXT_LINE_SPACING = 2.0f;         // 2px between lines (reduced from 3px)
    constexpr float DETAILS_TEXT_BG_ALPHA = 60.0f;            // ~24% opacity (very subtle)
    constexpr float DETAILS_TEXT_SHADOW_ALPHA = 180.0f;       // ~71% opacity (strong shadow)

    // Compact Summary Views (gear display, stat summaries)
    constexpr float SUMMARY_Y_OFFSET = 40.0f;                 // 40px below entity (reduced from 45px)
    constexpr float SUMMARY_BG_PADDING_X = 5.0f;              // 5px horizontal padding (increased from 4px)
    constexpr float SUMMARY_BG_PADDING_Y = 3.0f;              // 3px vertical padding (increased from 2px)
    constexpr float SUMMARY_BG_ROUNDING = 3.0f;               // 3px rounded corners
    constexpr float SUMMARY_BG_ALPHA = 70.0f;                 // ~27% opacity (increased from 60 for visibility)
    constexpr float SUMMARY_SHADOW_ALPHA = 180.0f;            // ~71% opacity (increased from 160)
    constexpr float SUMMARY_TEXT_ALPHA = 220.0f;              // ~86% opacity (increased from 200)

    // Text Rendering - Positioning (generic text system)
    constexpr float TEXT_ANCHOR_GAP = 5.0f;                   // 5px gap between anchor and text
    constexpr float TEXT_LINE_SPACING_EXTRA = 2.0f;           // 2px extra spacing between lines

    // Text Rendering - Default Style Values (fallback when not specified)
    constexpr float TEXT_DEFAULT_FONT_SIZE = 14.0f;           // 14px default (increased from 13px)
    constexpr float TEXT_DEFAULT_SHADOW_OFFSET_X = 1.0f;      // 1px horizontal shadow
    constexpr float TEXT_DEFAULT_SHADOW_OFFSET_Y = 1.0f;      // 1px vertical shadow
    constexpr float TEXT_DEFAULT_SHADOW_ALPHA = 128.0f;       // ~50% opacity (crisp shadow)
    constexpr float TEXT_DEFAULT_BG_PADDING_X = 4.0f;         // 4px horizontal padding
    constexpr float TEXT_DEFAULT_BG_PADDING_Y = 2.0f;         // 2px vertical padding
    constexpr float TEXT_DEFAULT_BG_ALPHA = 180.0f;           // ~71% opacity (visible background)
    constexpr float TEXT_DEFAULT_BG_ROUNDING = 3.0f;          // 3px rounded corners
    constexpr float TEXT_DEFAULT_BORDER_THICKNESS = 1.0f;     // 1px crisp border
    constexpr float TEXT_DEFAULT_LINE_SPACING = 2.0f;         // 2px between lines
}

/**
 * @brief Constants for the 3D Gadget Sphere/Gyroscope visual.
 */
namespace GadgetSphere {
    // LOD Transition
    constexpr float LOD_TRANSITION_START = 180.0f;
    constexpr float LOD_TRANSITION_END = 200.0f;

    // Geometry
    constexpr int NUM_RING_POINTS = 16;
    constexpr float VERTICAL_RADIUS = 0.35f;
    constexpr float HORIZONTAL_RADIUS_RATIO = 0.9f; // Horizontal radius = Vertical * this

    // Thickness
    constexpr float BASE_THICKNESS = 2.5f;
    constexpr float MIN_THICKNESS = 1.0f;
    constexpr float MAX_THICKNESS = 5.0f;
    constexpr float VERTICAL_THICKNESS_RATIO = 0.7f; // Vertical rings are thinner

    // Shading
    constexpr float DIM_COLOR_MULTIPLIER = 0.7f;

    // 2D Circle Fallback (Holographic Disc)
    constexpr float CIRCLE_RADIUS_BASE = 10.0f;
    constexpr float CIRCLE_RADIUS_MIN = 2.0f;
    constexpr float CIRCLE_RADIUS_MAX = 15.0f;
    constexpr float GLOW_ALPHA_RATIO = 0.3f;
    constexpr float CORE_ALPHA_RATIO = 0.7f;
}

} // namespace kx