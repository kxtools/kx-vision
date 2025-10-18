#pragma once

namespace kx {

    constexpr int CURRENT_SETTINGS_VERSION = 1;

    /**
     * @brief Display mode for player gear/equipment stats
     */
    enum class GearDisplayMode {
        Off = 0,        // No gear information displayed
        Compact = 1,    // Compact view: Show top 3 stat sets with percentages
        Attributes = 2, // Show top 3 dominant attributes with percentages
        Detailed = 3    // Detailed view: Full list of all gear slots and stats
    };

    /**
     * @brief Energy source to display for the player energy bar
     */
    enum class EnergyDisplayType {
        Dodge = 0,      // Player's dodge endurance
        Special = 1     // Mount or other special energy
    };

} // namespace kx
