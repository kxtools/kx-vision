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
    
    // NPC box - square for easy distinction
    constexpr float NPC_HEIGHT = 40.0f;
    constexpr float NPC_WIDTH = 40.0f;
    
    // Gadget box - very small square for minimal visual impact
    constexpr float GADGET_HEIGHT = 15.0f;
    constexpr float GADGET_WIDTH = 15.0f;
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