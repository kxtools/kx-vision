#pragma once

namespace kx {

    constexpr int CURRENT_SETTINGS_VERSION = 1;

    /**
     * @brief Display mode for player gear/equipment stats
     * Note: Use enableGearDisplay flag to enable/disable the feature
     */
    enum class GearDisplayMode {
        Compact = 0,    // Compact view: Show top 3 stat sets with percentages
        Attributes = 1, // Show top 3 dominant attributes with percentages
        Detailed = 2   // Detailed view: Full list of all gear slots and stats
    };

    /**
     * @brief Energy source to display for the player energy bar
     */
    enum class EnergyDisplayType {
        Endurance = 0,      // Player's dodge endurance
        Energy = 1     // Mount or other special energy
    };

    /**
     * @brief Display mode for movement trails
     */
    enum class TrailDisplayMode {
        Hostile = 0,  // Show only enemy trails (PvP/WvW)
        All = 1       // Show all player trails
    };

    /**
     * @brief Behavior for trail rendering at teleport points
     */
    enum class TrailTeleportMode {
        Tactical = 0,  // Break trail on teleport (default - clean visualization)
        Analysis = 1   // Connect with dotted line (cheat detection, portal tracking)
    };

} // namespace kx
