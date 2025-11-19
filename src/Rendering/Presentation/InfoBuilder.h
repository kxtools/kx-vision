#pragma once

#include <map>
#include <vector>
#include "../Data/RenderableData.h"
#include "../../Core/Settings.h"
#include "Generated/EnumsAndStructs.h"

namespace kx {

/**
 * @brief Unified builder for all entity information details
 * 
 * This class consolidates functionality from ESPPlayerDetailsBuilder and ESPEntityDetailsBuilder
 * to provide a single source for generating display information for all entity types.
 */
class InfoBuilder {
public:
    // ===== Player Methods =====
    
    /**
     * @brief Append basic player information details (name, level, profession, etc.)
     * @param player The player entity to build details for
     * @param settings Player ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @param out Output vector to append details to
     */
    static void AppendPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings, bool showDebugAddresses, std::vector<ColoredDetail>& out);

    /**
     * @brief Append detailed gear information showing each equipment slot and stat
     * @param player The player entity to analyze
     * @param out Output vector to append details to
     */
    static void AppendGearDetails(const RenderablePlayer* player, std::vector<ColoredDetail>& out);

    /**
     * @brief Build compact gear summary showing stat names and counts
     * @param player The player entity to analyze
     * @return Vector of compact stat info (stat name, count, highest rarity)
     */
    static std::vector<CompactStatInfo> BuildCompactGearSummary(const RenderablePlayer* player);

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

    // ===== NPC Methods =====
    
    /**
     * @brief Append NPC information details (name, level, health, attitude, rank)
     * @param npc The NPC entity to build details for
     * @param settings NPC ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @param out Output vector to append details to
     */
    static void AppendNpcDetails(const RenderableNpc* npc, const NpcEspSettings& settings, bool showDebugAddresses, std::vector<ColoredDetail>& out);

    // ===== Gadget Methods =====
    
    /**
     * @brief Append Gadget information details (type, resource node info, gatherable status)
     * @param gadget The gadget entity to build details for
     * @param settings Object ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @param out Output vector to append details to
     */
    static void AppendGadgetDetails(const RenderableGadget* gadget, const ObjectEspSettings& settings, bool showDebugAddresses, std::vector<ColoredDetail>& out);

    /**
     * @brief Append Attack Target information details (position, agent ID, type)
     * @param attackTarget The attack target entity to build details for
     * @param settings Object ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     * @param out Output vector to append details to
     */
    static void AppendAttackTargetDetails(const RenderableAttackTarget* attackTarget, const ObjectEspSettings& settings, bool showDebugAddresses, std::vector<ColoredDetail>& out);

private:
    /**
     * @brief Build attribute summary counting occurrences of each attribute
     * @param player The player entity to analyze
     * @return Map of attributes to their occurrence counts
     */
    static std::map<data::ApiAttribute, int> BuildAttributeSummary(const RenderablePlayer* player);
};

} // namespace kx

