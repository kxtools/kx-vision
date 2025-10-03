#pragma once

#include "../../../libs/ImGui/imgui.h"

namespace kx {

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
 * @brief Rendering effect constants
 * 
 * Constants that control visual effects and rendering behaviors in the ESP system.
 * These values tune the visual experience and performance characteristics.
 */
namespace RenderingEffects {
    // Distance fade zone - entities fade out in the last 11% of their distance limit
    // e.g., with 90m limit: fade starts at 80m (90 * 0.89), ends at 90m
    constexpr float FADE_ZONE_PERCENTAGE = 0.11f;
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
    constexpr unsigned int NPC_HOSTILE = IM_COL32(255, 80, 80, 210);      // Strong classic red
    constexpr unsigned int NPC_FRIENDLY = IM_COL32(100, 255, 100, 210);   // Bright classic green
    constexpr unsigned int NPC_NEUTRAL = IM_COL32(127, 255, 0, 210);      // Electric chartreuse (yellow-green)
    constexpr unsigned int NPC_INDIFFERENT = IM_COL32(240, 240, 240, 210); // Clean bright white
    constexpr unsigned int NPC_UNKNOWN = IM_COL32(255, 0, 255, 210);      // Magenta - debug/unknown
    
    // Gadget colors
    constexpr unsigned int GADGET = IM_COL32(255, 165, 80, 200);  // Warm orange/amber
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

    // Attached Health Bar (vertical bars attached to box sides)
    constexpr float ATTACHED_HEALTH_BAR_WIDTH = 4.0f;        // 4px width (visible but not obtrusive)
    constexpr float ATTACHED_HEALTH_BAR_SPACING = 2.0f;      // 2px gap from box edge
    constexpr float ATTACHED_HEALTH_BAR_BG_ALPHA = 150.0f;   // ~59% opacity for subtle background
    constexpr float ATTACHED_HEALTH_BAR_BORDER_ALPHA = 100.0f; // ~39% opacity for soft border

    // Standalone Health Bar (horizontal bars below entities)
    constexpr float STANDALONE_HEALTH_BAR_Y_OFFSET = 12.0f;          // 12px below entity (compact spacing)
    constexpr float STANDALONE_HEALTH_BAR_BG_ROUNDING = 1.0f;        // Subtle 1px rounding
    constexpr float STANDALONE_HEALTH_BAR_BORDER_ROUNDING = 1.0f;    // Matches background
    constexpr float STANDALONE_HEALTH_BAR_BORDER_THICKNESS = 1.0f;   // 1px crisp border
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

} // namespace kx