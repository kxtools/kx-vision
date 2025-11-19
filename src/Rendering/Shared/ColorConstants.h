#pragma once

#include "../../../libs/ImGui/imgui.h"

namespace kx {

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
    constexpr unsigned int FRIENDLY_NEUTRAL_BAR = IM_COL32(104, 197, 80, 255); // #68C550
    constexpr unsigned int NPC_HOSTILE = IM_COL32(220, 50, 40, 210);      // Thematic, high-contrast crimson
    constexpr unsigned int NPC_FRIENDLY = FRIENDLY_NEUTRAL_BAR;
    constexpr unsigned int NPC_NEUTRAL = FRIENDLY_NEUTRAL_BAR;
    constexpr unsigned int NPC_INDIFFERENT = IM_COL32(240, 240, 240, 210); // Clean bright white
    constexpr unsigned int NPC_UNKNOWN = IM_COL32(255, 0, 255, 210);      // Magenta - debug/unknown
    
    // Gadget colors
    constexpr unsigned int GADGET = IM_COL32(238, 221, 51, 255); // #EEDD33

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

} // namespace kx
