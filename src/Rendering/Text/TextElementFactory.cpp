#include "TextElementFactory.h"
#include "../Data/RenderableData.h"
#include "../Data/PlayerRenderData.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPStyling.h"
#include <sstream>
#include <iomanip>

namespace kx {

TextElement TextElementFactory::CreatePlayerName(const std::string& playerName, const glm::vec2& feetPos,
    unsigned int entityColor, float fadeAlpha, float fontSize) {
    TextElement element(playerName, feetPos, glm::vec2(0.0f, RenderingLayout::PLAYER_NAME_Y_OFFSET));
    element.SetStyle(GetPlayerNameStyle(fadeAlpha, entityColor, fontSize));
    return element;
}

TextElement TextElementFactory::CreateDistanceText(float distance, const glm::vec2& anchorPos, float fadeAlpha, float fontSize) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << distance << "m";
    
    TextElement element(oss.str(), anchorPos, glm::vec2(0.0f, -RenderingLayout::DISTANCE_TEXT_Y_OFFSET));
    element.SetStyle(GetDistanceStyle(fadeAlpha, fontSize));
    return element;
}

TextElement TextElementFactory::CreateDetailsText(const std::vector<ColoredDetail>& details,
                                                  const glm::vec2& anchorPos, float fadeAlpha, float fontSize) {
    if (details.empty()) {
        return TextElement("", anchorPos);
    }
    
    // Convert ColoredDetail to text lines with colors
    std::vector<std::vector<TextSegment>> lines;
    for (const auto& detail : details) {
        // ColoredDetail colors already have alpha = 255, they will be faded by the renderer
        lines.push_back({TextSegment(detail.text, detail.color)});
    }
    
    // Position below anchor with proper offset
    glm::vec2 adjustedAnchor(anchorPos.x, anchorPos.y + RenderingLayout::DETAILS_TEXT_Y_OFFSET);
    TextElement element(lines, adjustedAnchor, TextAnchor::Custom);
    
    TextStyle style = GetDetailsStyle(fadeAlpha, fontSize);
    element.SetStyle(style);
    element.SetLineSpacing(RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    
    return element;
}

TextElement TextElementFactory::CreateGearSummary(const std::vector<CompactStatInfo>& summary,
                                                  const glm::vec2& feetPos, float fadeAlpha, float fontSize) {
    if (summary.empty()) {
        return TextElement("", feetPos);
    }
    
    // Build multi-colored segments
    std::vector<TextSegment> segments;
    
    // Add prefix
    segments.push_back(TextSegment("Stats: ", IM_COL32(200, 210, 255, 255)));
    
    // Add each stat with its rarity color
    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        std::string segment = std::to_string(info.count) + "x " + info.statName;
        ImU32 rarityColor = ESPHelpers::GetRarityColor(info.highestRarity);
        segments.push_back(TextSegment(segment, rarityColor));
        
        // Add separator
        if (i < summary.size() - 1) {
            segments.push_back(TextSegment(", ", IM_COL32(200, 210, 255, 255)));
        }
    }
    
    // Position below feet with proper offset
    glm::vec2 adjustedPos(feetPos.x, feetPos.y + RenderingLayout::SUMMARY_Y_OFFSET);
    TextElement element(segments, adjustedPos, TextAnchor::Custom);
    
    TextStyle style = GetSummaryStyle(fadeAlpha, fontSize);
    style.useCustomTextColor = true;  // Enable per-segment colors
    element.SetStyle(style);
    
    return element;
}

TextElement TextElementFactory::CreateDominantStats(const std::vector<DominantStat>& stats,
                                                    const glm::vec2& feetPos, float fadeAlpha, float fontSize) {
    if (stats.empty()) {
        return TextElement("", feetPos);
    }
    
    // Build the display string
    std::string summaryText = "[";
    for (size_t i = 0; i < stats.size(); ++i) {
        summaryText += stats[i].name;
        if (i < stats.size() - 1) {
            summaryText += " | ";
        }
    }
    summaryText += "]";
    
    // Position below feet with proper offset
    glm::vec2 adjustedPos(feetPos.x, feetPos.y + RenderingLayout::SUMMARY_Y_OFFSET);
    TextElement element(summaryText, adjustedPos, TextAnchor::Custom);
    element.SetStyle(GetSummaryStyle(fadeAlpha, fontSize));
    return element;
}

TextStyle TextElementFactory::GetPlayerNameStyle(float fadeAlpha, unsigned int entityColor, float fontSize) { // Add fontSize
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    
    // Text - use entityColor directly (already has proper RGB values from ESPColors constants)
    // Just replace the alpha component with our text alpha
    unsigned int textAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_TEXT_ALPHA);
    style.textColor = (entityColor & 0x00FFFFFF) | (textAlpha << 24);
    
    // Shadow
    style.enableShadow = true;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::PLAYER_NAME_SHADOW_ALPHA / 255.0f;
    
    // Background
    style.enableBackground = true;
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
    
    // Text
    style.textColor = IM_COL32(255, 255, 255, static_cast<unsigned int>(RenderingLayout::DISTANCE_TEXT_TEXT_ALPHA));
    
    // Shadow
    style.enableShadow = true;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::DISTANCE_TEXT_SHADOW_ALPHA / 255.0f;
    
    // Background
    style.enableBackground = true;
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
    
    // Shadow
    style.enableShadow = true;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::DETAILS_TEXT_SHADOW_ALPHA / 255.0f;
    
    // Background
    style.enableBackground = true;
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
    
    // Text
    style.textColor = IM_COL32(200, 210, 255, static_cast<unsigned int>(RenderingLayout::SUMMARY_TEXT_ALPHA));
    
    // Shadow
    style.enableShadow = true;
    style.shadowOffset = ImVec2(RenderingLayout::TEXT_SHADOW_OFFSET, RenderingLayout::TEXT_SHADOW_OFFSET);
    style.shadowAlpha = RenderingLayout::SUMMARY_SHADOW_ALPHA / 255.0f;
    
    // Background
    style.enableBackground = true;
    style.backgroundPadding = ImVec2(RenderingLayout::SUMMARY_BG_PADDING_X, RenderingLayout::SUMMARY_BG_PADDING_Y);
    style.backgroundAlpha = RenderingLayout::SUMMARY_BG_ALPHA / 255.0f;
    style.backgroundRounding = RenderingLayout::SUMMARY_BG_ROUNDING;
    
    // No border
    style.enableBorder = false;
    
    return style;
}

} // namespace kx
