#include "TextElementFactory.h"
#include "../Data/RenderableData.h"
#include "../Data/PlayerRenderData.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPStyling.h"
#include "../Utils/ESPFormatting.h"
#include "../Utils/ColorConstants.h"
#include "../Layout/LayoutCalculator.h"
#include "../../Core/AppState.h"
#include "../../Utils/UnitConversion.h"
#include <sstream>
#include <iomanip>

namespace kx {

namespace {
    /**
     * @brief Format distance based on user's display mode preference
     * @param meters Distance in meters
     * @return Formatted distance string
     */
    std::string FormatDistance(float meters) {
        const auto& settings = AppState::Get().GetSettings();
        float units = UnitConversion::MetersToGW2Units(meters);
        
        std::ostringstream oss;
        oss << std::fixed;
        
        switch (settings.distance.displayMode) {
            case DistanceDisplayMode::Meters:
                oss << std::setprecision(1) << meters << "m";
                break;
            case DistanceDisplayMode::GW2Units:
                oss << std::setprecision(0) << units;
                break;
            case DistanceDisplayMode::Both:
                oss << std::setprecision(0) << units << " (" << std::setprecision(1) << meters << "m)";
                break;
        }
        
        return oss.str();
    }
}

TextElement TextElementFactory::CreatePlayerName(const std::string& playerName, const glm::vec2& feetPos,
    unsigned int entityColor, float fadeAlpha, float fontSize) {
    TextElement element(playerName, {0,0});
    element.SetStyle(GetPlayerNameStyle(fadeAlpha, entityColor, fontSize));
    return element;
}

TextElement TextElementFactory::CreateDistanceTextAt(float distance, const glm::vec2& position, float fadeAlpha, float fontSize) {
    std::string formattedDistance = FormatDistance(distance);
    
    TextElement element(formattedDistance, position, TextAnchor::AbsoluteTopLeft);
    element.SetStyle(GetDistanceStyle(fadeAlpha, fontSize));
    element.SetAlignment(TextAlignment::Center);
    return element;
}

TextElement TextElementFactory::CreateDetailsText(const std::vector<ColoredDetail>& details,
                                                  const glm::vec2& anchorPos, float fadeAlpha, float fontSize) {
    if (details.empty()) {
        return TextElement("", {0,0});
    }
    
    // Convert ColoredDetail to text lines with colors
    std::vector<std::vector<TextSegment>> lines;
    for (const auto& detail : details) {
        // ColoredDetail colors already have alpha = 255, they will be faded by the renderer
        lines.push_back({TextSegment(detail.text, detail.color)});
    }
    
    TextElement element(lines, {0,0});
    
    TextStyle style = GetDetailsStyle(fadeAlpha, fontSize);
    element.SetStyle(style);
    element.SetLineSpacing(RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    
    return element;
}

TextElement TextElementFactory::CreateGearSummary(const std::vector<CompactStatInfo>& summary,
                                                  const glm::vec2& feetPos, float fadeAlpha, float fontSize) {
    if (summary.empty()) {
        return TextElement("", {0,0});
    }
    
    // Build multi-colored segments
    std::vector<TextSegment> segments;
    
    // Add prefix
    segments.push_back(TextSegment("Stats: ", ESPColors::SUMMARY_TEXT_RGB));
    
    // Add each stat with its rarity color
    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << info.percentage << "% " << info.statName;
        std::string segment = oss.str();

        ImU32 rarityColor = ESPStyling::GetRarityColor(info.highestRarity);
        segments.push_back(TextSegment(segment, rarityColor));
        
        // Add separator
        if (i < summary.size() - 1) {
            segments.push_back(TextSegment(", ", ESPColors::SUMMARY_TEXT_RGB));
        }
    }
    
    TextElement element(segments, {0,0});
    
    TextStyle style = GetSummaryStyle(fadeAlpha, fontSize);
    style.useCustomTextColor = true;  // Enable per-segment colors
    element.SetStyle(style);
    
    return element;
}

TextElement TextElementFactory::CreateDominantStats(const std::vector<DominantStat>& stats,
                                                    Game::ItemRarity topRarity, // topRarity is no longer used for coloring, but kept for signature compatibility
                                                    const glm::vec2& feetPos, float fadeAlpha, float fontSize) {
    if (stats.empty()) {
        return TextElement("", {0,0});
    }

    // Build multi-colored segments for the summary
    std::vector<TextSegment> segments;
    segments.push_back(TextSegment("[", ESPColors::SUMMARY_TEXT_RGB));

    for (size_t i = 0; i < stats.size(); ++i) {
        const auto& stat = stats[i];

        // Format the string with name and percentage
        std::ostringstream oss;
        oss << stat.name << " " << std::fixed << std::setprecision(0) << stat.percentage << "%";
        
        // Add the segment with its specific tactical color
        segments.push_back(TextSegment(oss.str(), stat.color));

        // Add separator if not the last element
        if (i < stats.size() - 1) {
            segments.push_back(TextSegment(" | ", ESPColors::SUMMARY_TEXT_RGB));
        }
    }

    segments.push_back(TextSegment("]", ESPColors::SUMMARY_TEXT_RGB));
    
    TextElement element(segments, {0,0});

    TextStyle style = GetSummaryStyle(fadeAlpha, fontSize);
    style.useCustomTextColor = true; // IMPORTANT: Enable per-segment coloring
    element.SetStyle(style);
    
    return element;
}

TextElement TextElementFactory::CreateDamageNumber(const std::string& number, const glm::vec2& anchorPos, float fadeAlpha, float fontSize)
{
    // Anchor above the health bar with a small gap
    TextElement element(number, anchorPos, glm::vec2(0.0f, -5.0f)); 
    
    // Define a unique style for the damage number
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    style.textColor = IM_COL32(255, 255, 255, 255); // Full white
    // Shadow (respect global setting)
    const auto& settings = AppState::Get().GetSettings();
    style.enableShadow = settings.appearance.enableTextShadows;
    style.shadowAlpha = RenderingLayout::TEXT_SHADOW_ALPHA;
    style.enableBackground = false; // No background, just the number

    element.SetStyle(style);
    return element;
}

TextElement TextElementFactory::CreatePlayerNameAt(const std::string& playerName, const glm::vec2& position, 
    unsigned int entityColor, float fadeAlpha, float fontSize) {
    TextElement element(playerName, position, TextAnchor::AbsoluteTopLeft);
    element.SetStyle(GetPlayerNameStyle(fadeAlpha, entityColor, fontSize));
    element.SetAlignment(TextAlignment::Center);
    return element;
}

TextElement TextElementFactory::CreateDetailsTextAt(const std::vector<ColoredDetail>& details, const glm::vec2& position, 
                                                  float fadeAlpha, float fontSize) {
    if (details.empty()) {
        return TextElement("", position, TextAnchor::AbsoluteTopLeft);
    }
    
    std::vector<std::vector<TextSegment>> lines;
    for (const auto& detail : details) {
        lines.push_back({TextSegment(detail.text, detail.color)});
    }
    
    TextElement element(lines, position, TextAnchor::AbsoluteTopLeft);
    
    TextStyle style = GetDetailsStyle(fadeAlpha, fontSize);
    element.SetStyle(style);
    element.SetLineSpacing(RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    element.SetAlignment(TextAlignment::Center);
    
    return element;
}

TextElement TextElementFactory::CreateGearSummaryAt(const std::vector<CompactStatInfo>& summary, const glm::vec2& position, 
                                                  float fadeAlpha, float fontSize) {
    if (summary.empty()) {
        return TextElement("", position, TextAnchor::AbsoluteTopLeft);
    }
    
    std::vector<TextSegment> segments;
    segments.push_back(TextSegment("Stats: ", ESPColors::SUMMARY_TEXT_RGB));
    
    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << info.percentage << "% " << info.statName;
        std::string segment = oss.str();

        ImU32 rarityColor = ESPStyling::GetRarityColor(info.highestRarity);
        segments.push_back(TextSegment(segment, rarityColor));
        
        if (i < summary.size() - 1) {
            segments.push_back(TextSegment(", ", ESPColors::SUMMARY_TEXT_RGB));
        }
    }
    
    TextElement element(segments, position, TextAnchor::AbsoluteTopLeft);
    
    TextStyle style = GetSummaryStyle(fadeAlpha, fontSize);
    style.useCustomTextColor = true;
    element.SetStyle(style);
    element.SetAlignment(TextAlignment::Center);
    
    return element;
}

TextElement TextElementFactory::CreateDominantStatsAt(const std::vector<DominantStat>& stats, Game::ItemRarity topRarity, 
                                                    const glm::vec2& position, float fadeAlpha, float fontSize) {
    if (stats.empty()) {
        return TextElement("", position, TextAnchor::AbsoluteTopLeft);
    }

    std::vector<TextSegment> segments;
    segments.push_back(TextSegment("[", ESPColors::SUMMARY_TEXT_RGB));

    for (size_t i = 0; i < stats.size(); ++i) {
        const auto& stat = stats[i];

        std::ostringstream oss;
        oss << stat.name << " " << std::fixed << std::setprecision(0) << stat.percentage << "%";
        
        segments.push_back(TextSegment(oss.str(), stat.color));

        if (i < stats.size() - 1) {
            segments.push_back(TextSegment(" | ", ESPColors::SUMMARY_TEXT_RGB));
        }
    }

    segments.push_back(TextSegment("]", ESPColors::SUMMARY_TEXT_RGB));
    
    TextElement element(segments, position, TextAnchor::AbsoluteTopLeft);

    TextStyle style = GetSummaryStyle(fadeAlpha, fontSize);
    style.useCustomTextColor = true;
    element.SetStyle(style);
    element.SetAlignment(TextAlignment::Center);
    
    return element;
}

TextStyle TextElementFactory::GetPlayerNameStyle(float fadeAlpha, unsigned int entityColor, float fontSize) { // Add fontSize
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    
    // Get settings once for both shadow and background
    const auto& settings = AppState::Get().GetSettings();
    
    // Text - use entityColor directly (already has proper RGB values from ESPColors constants)
    // Just replace the alpha component with our text alpha
    unsigned int textAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_TEXT_ALPHA);
    style.textColor = (entityColor & 0x00FFFFFF) | (textAlpha << 24);
    
    // Shadow (respect global setting)
    style.enableShadow = settings.appearance.enableTextShadows;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::PLAYER_NAME_SHADOW_ALPHA / 255.0f;
    
    // Background (respect global setting)
    style.enableBackground = settings.appearance.enableTextBackgrounds;
    style.backgroundPadding = ImVec2(RenderingLayout::PLAYER_NAME_BG_PADDING_X, RenderingLayout::PLAYER_NAME_BG_PADDING_Y);
    style.backgroundAlpha = RenderingLayout::PLAYER_NAME_BG_ALPHA / 255.0f;
    style.backgroundRounding = RenderingLayout::PLAYER_NAME_BG_ROUNDING;
    
    // No border - keep it clean and natural like game UI
    style.enableBorder = false;
    
    return style;
}

TextStyle TextElementFactory::GetDistanceStyle(float fadeAlpha, float fontSize) {
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    
    // Get settings once for both shadow and background
    const auto& settings = AppState::Get().GetSettings();
    
    // Text
    style.textColor = IM_COL32(255, 255, 255, static_cast<unsigned int>(RenderingLayout::DISTANCE_TEXT_TEXT_ALPHA));
    
    // Shadow (respect global setting)
    style.enableShadow = settings.appearance.enableTextShadows;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::DISTANCE_TEXT_SHADOW_ALPHA / 255.0f;
    
    // Background (respect global setting)
    style.enableBackground = settings.appearance.enableTextBackgrounds;
    style.backgroundPadding = ImVec2(RenderingLayout::DISTANCE_TEXT_BG_PADDING_X, RenderingLayout::DISTANCE_TEXT_BG_PADDING_Y);
    style.backgroundAlpha = RenderingLayout::DISTANCE_TEXT_BG_ALPHA / 255.0f;
    style.backgroundRounding = RenderingLayout::DISTANCE_TEXT_BG_ROUNDING;
    
    // No border
    style.enableBorder = false;
    
    return style;
}

TextStyle TextElementFactory::GetDetailsStyle(float fadeAlpha, float fontSize) {
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    style.useCustomTextColor = true;  // Details have per-line colors
    
    // Get settings once for both shadow and background
    const auto& settings = AppState::Get().GetSettings();
    
    // Shadow (respect global setting)
    style.enableShadow = settings.appearance.enableTextShadows;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::DETAILS_TEXT_SHADOW_ALPHA / 255.0f;
    
    // Background (respect global setting)
    style.enableBackground = settings.appearance.enableTextBackgrounds;
    style.backgroundPadding = ImVec2(RenderingLayout::DETAILS_TEXT_BG_PADDING_X, RenderingLayout::DETAILS_TEXT_BG_PADDING_Y);
    style.backgroundAlpha = RenderingLayout::DETAILS_TEXT_BG_ALPHA / 255.0f;
    style.backgroundRounding = RenderingLayout::DETAILS_TEXT_BG_ROUNDING;
    
    // No border
    style.enableBorder = false;
    
    return style;
}

TextStyle TextElementFactory::GetSummaryStyle(float fadeAlpha, float fontSize) {
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    
    // Get settings once for both shadow and background
    const auto& settings = AppState::Get().GetSettings();
    
    // Text - use SUMMARY_TEXT_RGB base color with custom alpha
    style.textColor = (ESPColors::SUMMARY_TEXT_RGB & 0x00FFFFFF) | (static_cast<unsigned int>(RenderingLayout::SUMMARY_TEXT_ALPHA) << 24);
    
    // Shadow (respect global setting)
    style.enableShadow = settings.appearance.enableTextShadows;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::SUMMARY_SHADOW_ALPHA / 255.0f;
    
    // Background (respect global setting)
    style.enableBackground = settings.appearance.enableTextBackgrounds;
    style.backgroundPadding = ImVec2(RenderingLayout::SUMMARY_BG_PADDING_X, RenderingLayout::SUMMARY_BG_PADDING_Y);
    style.backgroundAlpha = RenderingLayout::SUMMARY_BG_ALPHA / 255.0f;
    style.backgroundRounding = RenderingLayout::SUMMARY_BG_ROUNDING;
    
    // No border
    style.enableBorder = false;
    
    return style;
}

TextElement TextElementFactory::CreateIdentityLine(const LayoutRequest& request, bool includeName, bool includeDistance) {
    if (!includeName && !includeDistance) {
        return TextElement("", {0,0});
    }

    const auto& entityContext = request.entityContext;
    const auto& props = request.visualProps;

    std::vector<TextSegment> segments;

    // Add the Name segment
    if (includeName) {
        std::string entityName = "";
        if (entityContext.entityType == ESPEntityType::Player) {
            entityName = entityContext.playerName;
            if (entityName.empty()) {
                const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
                const char* profName = ESPFormatting::GetProfessionName(player->profession);
                if (profName) entityName = profName;
            }
        }
        segments.push_back({ entityName, props.fadedEntityColor });
    }

    // Add the Separator and Distance segments
    if (includeDistance) {
        if (includeName) {
            segments.push_back({ " • ", ESPColors::DEFAULT_TEXT });
        }
        std::string formattedDistance = FormatDistance(entityContext.gameplayDistance);
        segments.push_back({ formattedDistance, ESPColors::DEFAULT_TEXT });
    }
    
    // Create and Style the TextElement
    TextElement element(segments, {0,0});
    TextStyle style = GetPlayerNameStyle(props.finalAlpha, props.fadedEntityColor, props.finalFontSize);
    style.useCustomTextColor = true;
    element.SetStyle(style);

    return element;
}

} // namespace kx
