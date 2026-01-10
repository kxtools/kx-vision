#pragma once

#include <utility>
#include "../../../Rendering/Data/RenderableData.h"
#include "../../../Core/Settings.h"
#include "../../../Game/Generated/EnumsAndStructs.h"

struct ImDrawList;
namespace kx {
    struct LayoutCursor;
    struct VisualProperties;
}

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
     * @brief Render basic player information details (name, level, profession, etc.)
     * @param drawList The ImGui draw list to render to
     * @param cursor Layout cursor for positioning
     * @param props Visual properties for styling
     * @param player The player entity to render details for
     * @param settings Player ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     */
    static void RenderPlayerDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderablePlayer* player, const PlayerEspSettings& settings, const AppearanceSettings& appearance, bool showDebugAddresses);

    /**
     * @brief Render detailed gear information showing each equipment slot and stat
     * @param drawList The ImGui draw list to render to
     * @param cursor Layout cursor for positioning
     * @param props Visual properties for styling
     * @param player The player entity to analyze
     */
    static void RenderGearDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderablePlayer* player, const AppearanceSettings& appearance);

    /**
     * @brief Build compact gear summary showing stat names and counts
     * @param player The player entity to analyze
     * @param outBuffer Output buffer to write results to
     * @param bufferSize Size of the output buffer (typically 3)
     * @return Number of stats written (0-3, max 3 after sorting)
     */
    static size_t BuildCompactGearSummary(const RenderablePlayer* player, CompactStatInfo* outBuffer, size_t bufferSize);

    /**
     * @brief Build top 3 dominant stats with percentages
     * @param player The player entity to analyze
     * @param outBuffer Output buffer to write results to
     * @param bufferSize Size of the output buffer (typically 3)
     * @return Number of stats written (0-3, max 3 after sorting)
     */
    static size_t BuildDominantStats(const RenderablePlayer* player, DominantStat* outBuffer, size_t bufferSize);

    /**
     * @brief Get the highest rarity of all equipped items for a player
     * @param player The player entity to analyze
     * @return The highest item rarity found
     */
    static Game::ItemRarity GetHighestRarity(const RenderablePlayer* player);

    // ===== NPC Methods =====
    
    /**
     * @brief Render NPC information details (name, level, health, attitude, rank)
     * @param drawList The ImGui draw list to render to
     * @param cursor Layout cursor for positioning
     * @param props Visual properties for styling
     * @param npc The NPC entity to render details for
     * @param settings NPC ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     */
    static void RenderNpcDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderableNpc* npc, const NpcEspSettings& settings, const AppearanceSettings& appearance, bool showDebugAddresses);

    // ===== Gadget Methods =====
    
    /**
     * @brief Render Gadget information details (type, resource node info, gatherable status)
     * @param drawList The ImGui draw list to render to
     * @param cursor Layout cursor for positioning
     * @param props Visual properties for styling
     * @param gadget The gadget entity to render details for
     * @param settings Object ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     */
    static void RenderGadgetDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderableGadget* gadget, const ObjectEspSettings& settings, const AppearanceSettings& appearance, bool showDebugAddresses);

    /**
     * @brief Render Attack Target information details (position, agent ID, type)
     * @param drawList The ImGui draw list to render to
     * @param cursor Layout cursor for positioning
     * @param props Visual properties for styling
     * @param attackTarget The attack target entity to render details for
     * @param settings Object ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     */
    static void RenderAttackTargetDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderableAttackTarget* attackTarget, const ObjectEspSettings& settings, const AppearanceSettings& appearance, bool showDebugAddresses);

    /**
     * @brief Render Item information details (item ID, rarity, position)
     * @param drawList The ImGui draw list to render to
     * @param cursor Layout cursor for positioning
     * @param props Visual properties for styling
     * @param item The item entity to render details for
     * @param settings Object ESP settings for filtering what to display
     * @param showDebugAddresses Whether to include memory addresses for debugging
     */
    static void RenderItemDetails(ImDrawList* drawList, LayoutCursor& cursor, const VisualProperties& props, const RenderableItem* item, const ObjectEspSettings& settings, const AppearanceSettings& appearance, bool showDebugAddresses);

private:
    /**
     * @brief Build attribute summary counting occurrences of each attribute
     * @param player The player entity to analyze
     * @param outBuffer Output buffer to write results to
     * @param bufferSize Size of the output buffer
     * @return Number of unique attributes found
     */
    static size_t BuildAttributeSummary(const RenderablePlayer* player, std::pair<kx::data::ApiAttribute, int>* outBuffer, size_t bufferSize);
};

} // namespace kx

