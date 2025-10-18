#pragma once

#include <map>
#include <vector>
#include "../Data/RenderableData.h"
#include "../../Core/Settings.h"
#include "Generated/EnumsAndStructs.h"

namespace kx {

/**
 * @brief Builds detailed text information for player entities
 * 
 * This class is responsible for generating all player-specific display information,
 * including basic details, gear analysis, stat summaries, and attribute breakdowns.
 * Extracted from ESPStageRenderer for better scalability and single responsibility.
 */
class ESPPlayerDetailsBuilder {
public:
    /**
     * @brief Build basic player information details (name, level, profession, etc.)
     * @param player The player entity to build details for
     * @param settings Player ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @return Vector of colored text details
     */
    static std::vector<ColoredDetail> BuildPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings, bool showDebugAddresses);

    /**
     * @brief Build detailed gear information showing each equipment slot and stat
     * @param player The player entity to analyze
     * @return Vector of colored text details (one per gear slot)
     */
    static std::vector<ColoredDetail> BuildGearDetails(const RenderablePlayer* player);

    /**
     * @brief Build compact gear summary showing stat names and counts
     * @param player The player entity to analyze
     * @return Vector of compact stat info (stat name, count, highest rarity)
     */
    static std::vector<CompactStatInfo> BuildCompactGearSummary(const RenderablePlayer* player);

    /**
     * @brief Build attribute summary counting occurrences of each attribute
     * @param player The player entity to analyze
     * @return Map of attributes to their occurrence counts
     */
    static std::map<data::ApiAttribute, int> BuildAttributeSummary(const RenderablePlayer* player);

    /**
     * @brief Build top 3 dominant stats with percentages
     * @param player The player entity to analyze
     * @return Vector of dominant stats (name and percentage)
     */
    static std::vector<DominantStat> BuildDominantStats(const RenderablePlayer* player);

    /**
     * @brief Get the highest rarity of all equipped items for a player
     * @param player The player entity to analyze
     * @return The highest item rarity found
     */
    static Game::ItemRarity GetHighestRarity(const RenderablePlayer* player);
};

} // namespace kx
