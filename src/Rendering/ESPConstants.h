#pragma once

#include "../../libs/ImGui/imgui.h"

namespace kx {

/**
 * @brief Box dimension constants for different entity types
 * 
 * These constants define the visual bounding box sizes for each entity type
 * in the ESP system, scaled by distance for proper perspective.
 */
namespace BoxDimensions {
    // Player box - tall rectangle for humanoid shape
    constexpr float PLAYER_HEIGHT = 50.0f;
    constexpr float PLAYER_WIDTH = 30.0f;
    
    // NPC box - smaller square for less visual clutter
    constexpr float NPC_HEIGHT = 30.0f;
    constexpr float NPC_WIDTH = 30.0f;
    
    // Gadget box - very small square for minimal visual impact
    constexpr float GADGET_HEIGHT = 10.0f;
    constexpr float GADGET_WIDTH = 10.0f;
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
    // Player colors
    constexpr unsigned int PLAYER = IM_COL32(100, 200, 255, 220);  // Bright cyan/blue
    
    // NPC colors based on attitude
    constexpr unsigned int NPC_HOSTILE = IM_COL32(255, 80, 80, 210);      // Red - enemies
    constexpr unsigned int NPC_FRIENDLY = IM_COL32(100, 255, 100, 210);   // Green - allies
    constexpr unsigned int NPC_NEUTRAL = IM_COL32(255, 255, 100, 210);    // Yellow - neutral
    constexpr unsigned int NPC_INDIFFERENT = IM_COL32(180, 180, 180, 210); // Gray - indifferent
    constexpr unsigned int NPC_UNKNOWN = IM_COL32(255, 0, 255, 210);      // Magenta - debug/unknown
    
    // Gadget colors
    constexpr unsigned int GADGET = IM_COL32(255, 165, 80, 200);  // Warm orange/amber
}

} // namespace kx