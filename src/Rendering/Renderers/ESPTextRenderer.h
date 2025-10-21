#pragma once

#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include <string>
#include <vector>
#include "../Data/RenderableData.h"

namespace kx {

/**
 * @brief Utility functions for rendering text elements in ESP
 * 
 * This class handles all text-based rendering including player names, distances,
 * details, gear summaries, and stat displays. Separated for better organization.
 */
class ESPTextRenderer {
public:
    /**
     * @brief Render a player name below an entity
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param playerName Player name to display
     * @param entityColor Entity color (contains alpha for distance fading)
     * @param fontSize Font size to use
     */
    static void RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos,
                                const std::string& playerName, unsigned int entityColor, float fontSize);
    static void RenderPlayerNameAt(ImDrawList* drawList, const glm::vec2& position, const std::string& playerName, unsigned int entityColor, float fontSize);

    /**
     * @brief Render distance text above an entity
     * @param drawList ImGui draw list for rendering
     * @param center Center position of the entity
     * @param boxMin Upper-left corner of the entity bounding box
     * @param distance Distance value in meters
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     * @param fontSize Font size to use
     */
    static void RenderDistanceTextAt(ImDrawList* drawList, const glm::vec2& position, float distance, float fadeAlpha, float fontSize);

    /**
     * @brief Render details text below an entity
     * @param drawList ImGui draw list for rendering
     * @param center Center position of the entity
     * @param boxMax Lower-right corner of the entity bounding box
     * @param details Vector of detail strings to display (with colors)
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     * @param fontSize Font size to use
     */
    static void RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax,
                                 const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize);
    static void RenderDetailsTextAt(ImDrawList* drawList, const glm::vec2& position, const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize);

    /**
     * @brief Render a compact gear summary below a player name
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param summary Vector of compact stat info to display
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     * @param fontSize Font size to use
     */
    static void RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
                                 const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize);
    static void RenderGearSummaryAt(ImDrawList* drawList, const glm::vec2& position, const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize);

    /**
     * @brief Render dominant stats display below a player
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param stats Vector of dominant stats to display
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     * @param fontSize Font size to use
     */
    static void RenderDominantStats(ImDrawList* drawList, const glm::vec2& feetPos,
                                   const std::vector<DominantStat>& stats,
                                   Game::ItemRarity topRarity,
                                   float fadeAlpha, float fontSize);
    static void RenderDominantStatsAt(ImDrawList* drawList, const glm::vec2& position, const std::vector<DominantStat>& stats, Game::ItemRarity topRarity, float fadeAlpha, float fontSize);
};

} // namespace kx
