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
 */
namespace ESPColors {
    // Colors are defined in (Blue, Green, Red, Alpha) order for IM_COL32.

    // Default text color (used across all detail rendering)
    constexpr unsigned int DEFAULT_TEXT = IM_COL32(255, 255, 255, 255);  // White (R:255, G:255, B:255)

    // Player colors
    constexpr unsigned int PLAYER = IM_COL32(255, 200, 100, 220);  // Bright cyan/blue (R:100, G:200, B:255)
    
    // NPC colors based on attitude
    constexpr unsigned int NPC_HOSTILE = IM_COL32(80, 80, 255, 210);      // Red - enemies (R:255, G:80, B:80)
    constexpr unsigned int NPC_FRIENDLY = IM_COL32(100, 255, 100, 210);   // Green - allies (R:100, G:255, B:100)
    constexpr unsigned int NPC_NEUTRAL = IM_COL32(100, 255, 255, 210);    // Yellow - neutral (R:255, G:255, B:100)
    constexpr unsigned int NPC_INDIFFERENT = IM_COL32(180, 180, 180, 210); // Gray - indifferent (R:180, G:180, B:180)
    constexpr unsigned int NPC_UNKNOWN = IM_COL32(255, 0, 255, 210);      // Magenta - debug/unknown (R:255, G:0, B:255)
    
    // Gadget colors
    constexpr unsigned int GADGET = IM_COL32(80, 165, 255, 200);  // Warm orange/amber (R:255, G:165, B:80)
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
    constexpr float PLAYER_NAME_BG_ALPHA = 100.0f;
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
    constexpr float DISTANCE_TEXT_BG_ALPHA = 100.0f;
    constexpr float DISTANCE_TEXT_SHADOW_ALPHA = 180.0f;
    constexpr float DISTANCE_TEXT_TEXT_ALPHA = 220.0f;

    // Details Text
    constexpr float DETAILS_TEXT_Y_OFFSET = 5.0f;
    constexpr float DETAILS_TEXT_BG_PADDING_X = 3.0f;
    constexpr float DETAILS_TEXT_BG_PADDING_Y = 1.0f;
    constexpr float DETAILS_TEXT_BG_ROUNDING = 1.0f;
    constexpr float DETAILS_TEXT_LINE_SPACING = 3.0f;
    constexpr float DETAILS_TEXT_BG_ALPHA = 100.0f;
    constexpr float DETAILS_TEXT_SHADOW_ALPHA = 180.0f;

    // Compact Summary Views (Gear, Stats)
    constexpr float SUMMARY_Y_OFFSET = 45.0f;
    constexpr float SUMMARY_BG_PADDING_X = 4.0f;
    constexpr float SUMMARY_BG_PADDING_Y = 2.0f;
    constexpr float SUMMARY_BG_ROUNDING = 3.0f;
    constexpr float SUMMARY_BG_ALPHA = 80.0f;
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