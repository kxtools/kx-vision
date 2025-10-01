#pragma once

#include "glm.hpp"
#include "../../libs/ImGui/imgui.h"
#include <string>
#include <vector>

#include "RenderableData.h"

namespace kx {

/**
 * @brief Static utility class for rendering individual ESP features
 * 
 * This class contains static methods for rendering specific ESP visual elements
 * such as health bars, bounding boxes, text elements, and dots. Each method
 * is focused on a single rendering feature for better modularity and reusability.
 */
class ESPFeatureRenderer {
public:
    /**
     * @brief Render a health bar attached to the side of a bounding box
     * @param drawList ImGui draw list for rendering
     * @param boxMin Upper-left corner of the entity bounding box
     * @param boxMax Lower-right corner of the entity bounding box
     * @param healthPercent Health percentage (0.0-1.0)
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     */
    static void RenderAttachedHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, 
                                       float healthPercent, float fadeAlpha);

    /**
     * @brief Render a standalone health bar below an entity
     * @param drawList ImGui draw list for rendering
     * @param centerPos Center position of the entity
     * @param healthPercent Health percentage (0.0-1.0)
     * @param entityColor Entity color (used for border styling)
     */
    static void RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos, 
                                         float healthPercent, unsigned int entityColor, float barWidth, float barHeight);

    /**
     * @brief Render a player name below an entity
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param playerName Player name to display
     * @param entityColor Entity color (used for styling)
     */
    static void RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos, 
                                const std::string& playerName, unsigned int entityColor, float fontSize);

    /**
     * @brief Render a compact gear summary below a player name
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param summary The gear summary string to display
     * @param entityColor Entity color (used for styling)
     */
    static void RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
        const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize);

    static void RenderDominantStats(ImDrawList* drawList, const glm::vec2& feetPos,
        const std::vector<DominantStat>& stats, float fadeAlpha, float fontSize);

    /**
     * @brief Render a bounding box with corner indicators
     * @param drawList ImGui draw list for rendering
     * @param boxMin Upper-left corner of the bounding box
     * @param boxMax Lower-right corner of the bounding box
     * @param color Box color with alpha
     */
    static void RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, 
                                 unsigned int color, float thickness);

    /**
     * @brief Render distance text above an entity
     * @param drawList ImGui draw list for rendering
     * @param center Center position of the entity
     * @param boxMin Upper-left corner of the entity bounding box
     * @param distance Distance value in meters
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     */
    static void RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, 
                                  float distance, float fadeAlpha, float fontSize);

    /**
     * @brief Render a colored center dot for an entity
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param color Dot color with alpha
     */
    static void RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos, unsigned int color, float radius);

    /**
     * @brief Render a natural white dot (for gadgets)
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     */
    static void RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos, float fadeAlpha, float radius);

    /**
     * @brief Render details text below an entity
     * @param drawList ImGui draw list for rendering
     * @param center Center position of the entity
     * @param boxMax Lower-right corner of the entity bounding box
     * @param details Vector of detail strings to display
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     */
    static void RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, 
                                 const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize);

    /**
     * @brief Apply alpha multiplier to a color while preserving RGB values
     * @param color Original color (RGBA format)
     * @param alpha Alpha multiplier (0.0-1.0)
     * @return Modified color with adjusted alpha
     */
    static unsigned int ApplyAlphaToColor(unsigned int color, float alpha);
};

} // namespace kx