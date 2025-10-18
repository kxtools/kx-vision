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

/**
 * @brief Helper factory functions for creating common text elements
 * 
 * These helpers make it easy to migrate from the old rendering system
 * and provide convenient ways to create styled text elements.
 */
class TextElementFactory {
public:
    /**
     * @brief Create a player name text element (styled with background and border)
     * @param playerName The player's name
     * @param feetPos Position at player's feet
     * @param entityColor Color for the border
     * @param fadeAlpha Distance-based fade
     * @param fontSize Font size to use
     * @return Styled text element
     */
    static TextElement CreatePlayerName(const std::string& playerName, const glm::vec2& feetPos,
        unsigned int entityColor, float fadeAlpha, float fontSize);
    static TextElement CreatePlayerNameAt(const std::string& playerName, const glm::vec2& position, unsigned int entityColor, float fadeAlpha, float fontSize);
    
    /**
     * @brief Create a distance text element (shown above entity)
     * @param distance Distance in meters
     * @param anchorPos Position to anchor to (typically box top)
     * @param fadeAlpha Distance-based fade
     * @param fontSize Font size to use
     * @return Styled text element
     */
    static TextElement CreateDistanceText(float distance, const glm::vec2& anchorPos, float fadeAlpha, float fontSize);
    static TextElement CreateDistanceTextAt(float distance, const glm::vec2& position, float fadeAlpha, float fontSize);
    
    /**
     * @brief Create a details text element (multi-line colored details)
     * @param details Vector of colored details
     * @param anchorPos Position to anchor to (typically box bottom)
     * @param fadeAlpha Distance-based fade
     * @param fontSize Font size to use
     * @return Styled text element
     */
    static TextElement CreateDetailsText(const std::vector<ColoredDetail>& details,
                                        const glm::vec2& anchorPos, float fadeAlpha, float fontSize);
    static TextElement CreateDetailsTextAt(const std::vector<ColoredDetail>& details, const glm::vec2& position, float fadeAlpha, float fontSize);
    
    /**
     * @brief Create a gear summary text element (multi-colored stat summary)
     * @param summary Compact stat info
     * @param feetPos Position at player's feet
     * @param fadeAlpha Distance-based fade
     * @param fontSize Font size to use
     * @return Styled text element
     */
    static TextElement CreateGearSummary(const std::vector<CompactStatInfo>& summary,
                                        const glm::vec2& feetPos, float fadeAlpha, float fontSize);
    static TextElement CreateGearSummaryAt(const std::vector<CompactStatInfo>& summary, const glm::vec2& position, float fadeAlpha, float fontSize);
    
    /**
     * @brief Create a dominant stats text element
     * @param stats Dominant stat information
     * @param feetPos Position at player's feet
     * @param fadeAlpha Distance-based fade
     * @param fontSize Font size to use
     * @return Styled text element
     */
    static TextElement CreateDominantStats(const std::vector<DominantStat>& stats,
                                          Game::ItemRarity topRarity,
                                          const glm::vec2& feetPos, float fadeAlpha, float fontSize);
    static TextElement CreateDominantStatsAt(const std::vector<DominantStat>& stats, Game::ItemRarity topRarity, const glm::vec2& position, float fadeAlpha, float fontSize);
    
    static TextElement CreateDamageNumber(const std::string& number, const glm::vec2& anchorPos, float fadeAlpha, float fontSize);

    /**
     * @brief Get default style for player names
     */
    static TextStyle GetPlayerNameStyle(float fadeAlpha, unsigned int entityColor, float fontSize);
    
    /**
     * @brief Get default style for distance text
     */
    static TextStyle GetDistanceStyle(float fadeAlpha, float fontSize);
    
    /**
     * @brief Get default style for details text
     */
    static TextStyle GetDetailsStyle(float fadeAlpha, float fontSize);
    
    /**
     * @brief Get default style for gear summary
     */
    static TextStyle GetSummaryStyle(float fadeAlpha, float fontSize);
};

} // namespace kx
