#pragma once

#include "../../../libs/ImGui/imgui.h"

namespace kx {

/**
 * @brief Minimum size constraints for entity visibility
 * 
 * These constants ensure that entities remain visible even at extreme distances
 * by enforcing minimum pixel dimensions for each entity type.
 */
namespace MinimumSizes {
    // Player minimum sizes
    constexpr float PLAYER_MIN_HEIGHT = 20.0f;
    constexpr float PLAYER_MIN_WIDTH = 12.0f;
    
    // NPC minimum sizes (square boxes)
    constexpr float NPC_MIN_HEIGHT = 15.0f;
    constexpr float NPC_MIN_WIDTH = 15.0f;
    
    // Gadget minimum sizes (very small but still visible)
    constexpr float GADGET_MIN_HEIGHT = 3.0f;
    constexpr float GADGET_MIN_WIDTH = 3.0f;
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
 * Centralizes hardcoded pixel values for offsets, padding, and sizes to allow
 * for easy visual tuning of the ESP's appearance.
 */
namespace RenderingLayout {
    // Common
    constexpr float TEXT_SHADOW_OFFSET = 1.0f;
    constexpr float DOT_RADIUS_MULTIPLIER = 0.8f;

    // Attached Health Bar
    constexpr float ATTACHED_HEALTH_BAR_WIDTH = 4.0f;
    constexpr float ATTACHED_HEALTH_BAR_SPACING = 2.0f;
    constexpr float ATTACHED_HEALTH_BAR_BG_ALPHA = 150.0f;
    constexpr float ATTACHED_HEALTH_BAR_BORDER_ALPHA = 100.0f;

    // Standalone Health Bar
    constexpr float STANDALONE_HEALTH_BAR_Y_OFFSET = 15.0f;
    constexpr float STANDALONE_HEALTH_BAR_BG_ROUNDING = 1.0f;
    constexpr float STANDALONE_HEALTH_BAR_BORDER_ROUNDING = 1.0f;
    constexpr float STANDALONE_HEALTH_BAR_BORDER_THICKNESS = 1.0f;
    constexpr float STANDALONE_HEALTH_BAR_BG_ALPHA = 180.0f;
    constexpr float STANDALONE_HEALTH_BAR_HEALTH_ALPHA = 220.0f;
    constexpr float STANDALONE_HEALTH_BAR_BORDER_ALPHA = 100.0f;

    // Player Name
    constexpr float PLAYER_NAME_Y_OFFSET = 25.0f;
    constexpr float PLAYER_NAME_BG_PADDING_X = 4.0f;
    constexpr float PLAYER_NAME_BG_PADDING_Y = 2.0f;
    constexpr float PLAYER_NAME_BG_ROUNDING = 3.0f;
    constexpr float PLAYER_NAME_BORDER_THICKNESS = 1.0f;
    constexpr float PLAYER_NAME_BG_ALPHA = 60.0f;  // Very gentle, transparent background
    constexpr float PLAYER_NAME_BORDER_ALPHA = 120.0f;
    constexpr float PLAYER_NAME_SHADOW_ALPHA = 180.0f;
    constexpr float PLAYER_NAME_TEXT_ALPHA = 220.0f;

    // Bounding Box
    constexpr float BOX_CORNER_SIZE_MULTIPLIER = 4.0f;

    // Distance Text
    constexpr float DISTANCE_TEXT_Y_OFFSET = 5.0f;
    constexpr float DISTANCE_TEXT_BG_PADDING_X = 2.0f;
    constexpr float DISTANCE_TEXT_BG_PADDING_Y = 1.0f;
    constexpr float DISTANCE_TEXT_BG_ROUNDING = 2.0f;
    constexpr float DISTANCE_TEXT_BG_ALPHA = 60.0f;  // Gentle, transparent background
    constexpr float DISTANCE_TEXT_SHADOW_ALPHA = 180.0f;
    constexpr float DISTANCE_TEXT_TEXT_ALPHA = 220.0f;

    // Details Text
    constexpr float DETAILS_TEXT_Y_OFFSET = 5.0f;
    constexpr float DETAILS_TEXT_BG_PADDING_X = 3.0f;
    constexpr float DETAILS_TEXT_BG_PADDING_Y = 1.0f;
    constexpr float DETAILS_TEXT_BG_ROUNDING = 1.0f;
    constexpr float DETAILS_TEXT_LINE_SPACING = 3.0f;
    constexpr float DETAILS_TEXT_BG_ALPHA = 60.0f;  // Gentle, transparent background
    constexpr float DETAILS_TEXT_SHADOW_ALPHA = 180.0f;

    // Compact Summary Views (Gear, Stats)
    constexpr float SUMMARY_Y_OFFSET = 45.0f;
    constexpr float SUMMARY_BG_PADDING_X = 4.0f;
    constexpr float SUMMARY_BG_PADDING_Y = 2.0f;
    constexpr float SUMMARY_BG_ROUNDING = 3.0f;
    constexpr float SUMMARY_BG_ALPHA = 60.0f;  // Gentle, transparent background
    constexpr float SUMMARY_SHADOW_ALPHA = 160.0f;
    constexpr float SUMMARY_TEXT_ALPHA = 200.0f;

    // Text Rendering - Positioning
    constexpr float TEXT_ANCHOR_GAP = 5.0f;          // Gap between anchor and text (Above/Below modes)
    constexpr float TEXT_LINE_SPACING_EXTRA = 2.0f; // Extra spacing between multi-line text

    // Text Rendering - Default Style Values
    constexpr float TEXT_DEFAULT_FONT_SIZE = 13.0f;
    constexpr float TEXT_DEFAULT_SHADOW_OFFSET_X = 1.0f;
    constexpr float TEXT_DEFAULT_SHADOW_OFFSET_Y = 1.0f;
    constexpr float TEXT_DEFAULT_SHADOW_ALPHA = 128.0f;     // 0-255 range
    constexpr float TEXT_DEFAULT_BG_PADDING_X = 4.0f;
    constexpr float TEXT_DEFAULT_BG_PADDING_Y = 2.0f;
    constexpr float TEXT_DEFAULT_BG_ALPHA = 180.0f;         // 0-255 range
    constexpr float TEXT_DEFAULT_BG_ROUNDING = 3.0f;
    constexpr float TEXT_DEFAULT_BORDER_THICKNESS = 1.0f;
    constexpr float TEXT_DEFAULT_LINE_SPACING = 2.0f;
}

} // namespace kx