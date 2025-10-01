#pragma once

#include "../../libs/ImGui/imgui.h"

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

} // namespace kx