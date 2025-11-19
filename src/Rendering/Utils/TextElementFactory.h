#pragma once

#include "../Data/TextElement.h"
#include <vector>
#include <string>

#include "../../Game/GameEnums.h"

namespace kx {

// Forward declarations
struct ColoredDetail;
struct CompactStatInfo;
struct DominantStat;
struct EntityRenderContext;
struct VisualProperties;
struct FrameContext;
struct Settings;

// LayoutRequest structure (used by CreateIdentityLine)
struct LayoutRequest {
    const EntityRenderContext& entityContext;
    const VisualProperties& visualProps;
    const FrameContext& frameContext;
};

/**
 * @brief Helper factory functions for creating common text elements.
 * Optimized for the LayoutCursor system.
 */
class TextElementFactory {
public:
    /**
     * @brief Create a details text element (multi-line colored details) at a specific position
     */
    static TextElement CreateDetailsTextAt(const std::vector<ColoredDetail>& details, const glm::vec2& position, float fadeAlpha, float fontSize, const Settings& settings);
    
    /**
     * @brief Create a gear summary text element at a specific position
     */
    static TextElement CreateGearSummaryAt(const std::vector<CompactStatInfo>& summary, const glm::vec2& position, float fadeAlpha, float fontSize, const Settings& settings);
    
    /**
     * @brief Create a dominant stats text element at a specific position
     */
    static TextElement CreateDominantStatsAt(const std::vector<DominantStat>& stats, Game::ItemRarity topRarity, const glm::vec2& position, float fadeAlpha, float fontSize, const Settings& settings);
    
    static TextElement CreateDamageNumber(const std::string& number, const glm::vec2& anchorPos, float fadeAlpha, float fontSize, const Settings& settings);

    /**
     * @brief Create a merged identity line (name + distance) text element
     * @param request The layout request containing entity and visual data
     * @param includeName Whether to include the entity name
     * @param includeDistance Whether to include the distance
     * @return Styled text element with merged name and distance
     */
    static TextElement CreateIdentityLine(const LayoutRequest& request, bool includeName, bool includeDistance);

    /**
     * @brief Get default style for player names
     */
    static TextStyle GetPlayerNameStyle(float fadeAlpha, unsigned int entityColor, float fontSize, const Settings& settings);
    
    /**
     * @brief Get default style for distance text
     */
    static TextStyle GetDistanceStyle(float fadeAlpha, float fontSize, const Settings& settings);
    
    /**
     * @brief Get default style for details text
     */
    static TextStyle GetDetailsStyle(float fadeAlpha, float fontSize, const Settings& settings);
    
    /**
     * @brief Get default style for gear summary
     */
    static TextStyle GetSummaryStyle(float fadeAlpha, float fontSize, const Settings& settings);
};

} // namespace kx
