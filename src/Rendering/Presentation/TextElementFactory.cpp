#include "TextElementFactory.h"
#include "../Data/RenderableData.h"
#include "../Data/PlayerRenderData.h"
#include "../Data/EntityRenderContext.h"
#include "../Data/FrameData.h"
#include "../Shared/Constants.h"
#include "../../Core/Settings.h"
#include "../../Utils/UnitConversion.h"
#include <sstream>
#include <iomanip>
#include <format>

#include "Styling.h"
#include "Shared/Formatting.h"

namespace kx {

namespace {
    /**
     * @brief Format to a temporary string without streams
     * @param fmt Format string
     * @param args Format arguments
     * @return Formatted string
     */
    template<typename... Args>
    std::string FormatTemp(std::format_string<Args...> fmt, Args&&... args) {
        char buffer[kMaxTextBufferSize];
        // Write to buffer - 1 to guarantee space for null terminator
        auto result = std::format_to_n(buffer, kMaxTextBufferSize - 1, fmt, std::forward<Args>(args)...);
        *result.out = '\0';
        return std::string(buffer);
    }

    /**
     * @brief Format distance based on user's display mode preference
     * @param meters Distance in meters
     * @param settings Settings reference for display mode
     * @return Formatted distance string
     */
    std::string FormatDistance(float meters, const Settings& settings) {
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

TextElement TextElementFactory::CreateDetailsTextAt(const std::vector<ColoredDetail>& details, const glm::vec2& position, 
                                                  float fadeAlpha, float fontSize, const Settings& settings) {
    if (details.empty()) {
        return TextElement("", position, TextAnchor::AbsoluteTopLeft);
    }
    
    std::vector<std::vector<TextSegment>> lines;
    for (const auto& detail : details) {
        lines.push_back({TextSegment(detail.text.c_str(), detail.color)});
    }
    
    TextElement element(lines, position, TextAnchor::AbsoluteTopLeft);
    
    TextStyle style = GetDetailsStyle(fadeAlpha, fontSize, settings);
    element.SetStyle(style);
    element.SetLineSpacing(RenderingLayout::DETAILS_TEXT_LINE_SPACING);
    element.SetAlignment(TextAlignment::Center);
    
    return element;
}

TextElement TextElementFactory::CreateGearSummaryAt(const std::vector<CompactStatInfo>& summary, const glm::vec2& position, 
                                                  float fadeAlpha, float fontSize, const Settings& settings) {
    if (summary.empty()) {
        return TextElement("", position, TextAnchor::AbsoluteTopLeft);
    }
    
    std::vector<TextSegment> segments;
    segments.push_back(TextSegment("Stats: ", ESPColors::SUMMARY_TEXT_RGB));
    
    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        
        std::string segment = FormatTemp("{:.0f}% {}", info.percentage, info.statName);

        ImU32 rarityColor = Styling::GetRarityColor(info.highestRarity);
        segments.push_back(TextSegment(segment, rarityColor));
        
        if (i < summary.size() - 1) {
            segments.push_back(TextSegment(", ", ESPColors::SUMMARY_TEXT_RGB));
        }
    }
    
    TextElement element(segments, position, TextAnchor::AbsoluteTopLeft);
    
    TextStyle style = GetSummaryStyle(fadeAlpha, fontSize, settings);
    style.useCustomTextColor = true;
    element.SetStyle(style);
    element.SetAlignment(TextAlignment::Center);
    
    return element;
}

TextElement TextElementFactory::CreateDominantStatsAt(const std::vector<DominantStat>& stats, Game::ItemRarity topRarity, 
                                                    const glm::vec2& position, float fadeAlpha, float fontSize, const Settings& settings) {
    if (stats.empty()) {
        return TextElement("", position, TextAnchor::AbsoluteTopLeft);
    }

    std::vector<TextSegment> segments;
    segments.push_back(TextSegment("[", ESPColors::SUMMARY_TEXT_RGB));

    for (size_t i = 0; i < stats.size(); ++i) {
        const auto& stat = stats[i];

        std::string segment = FormatTemp("{} {:.0f}%", stat.name, stat.percentage);
        
        segments.push_back(TextSegment(segment, stat.color));

        if (i < stats.size() - 1) {
            segments.push_back(TextSegment(" | ", ESPColors::SUMMARY_TEXT_RGB));
        }
    }

    segments.push_back(TextSegment("]", ESPColors::SUMMARY_TEXT_RGB));
    
    TextElement element(segments, position, TextAnchor::AbsoluteTopLeft);

    TextStyle style = GetSummaryStyle(fadeAlpha, fontSize, settings);
    style.useCustomTextColor = true;
    element.SetStyle(style);
    element.SetAlignment(TextAlignment::Center);
    
    return element;
}

TextElement TextElementFactory::CreateDamageNumber(const std::string& number, const glm::vec2& anchorPos, float fadeAlpha, float fontSize, const Settings& settings)
{
    // Anchor above the health bar with a small gap
    TextElement element(number, anchorPos, glm::vec2(0.0f, -5.0f)); 
    
    // Define a unique style for the damage number
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    style.textColor = IM_COL32(255, 255, 255, 255); // Full white
    // Shadow (respect global setting)
    style.enableShadow = settings.appearance.enableTextShadows;
    style.shadowAlpha = RenderingLayout::TEXT_SHADOW_ALPHA;
    style.enableBackground = false; // No background, just the number

    element.SetStyle(style);
    return element;
}

TextStyle TextElementFactory::GetPlayerNameStyle(float fadeAlpha, unsigned int entityColor, float fontSize, const Settings& settings) {
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    
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

TextStyle TextElementFactory::GetDistanceStyle(float fadeAlpha, float fontSize, const Settings& settings) {
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    
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

TextStyle TextElementFactory::GetDetailsStyle(float fadeAlpha, float fontSize, const Settings& settings) {
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    style.useCustomTextColor = true;  // Details have per-line colors
    
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

TextStyle TextElementFactory::GetSummaryStyle(float fadeAlpha, float fontSize, const Settings& settings) {
    TextStyle style;
    style.fontSize = fontSize;
    style.fadeAlpha = fadeAlpha;
    
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
        if (entityContext.entityType == EntityTypes::Player) {
            entityName = entityContext.playerName;
            if (entityName.empty()) {
                const auto* player = static_cast<const RenderablePlayer*>(entityContext.entity);
                const char* profName = Formatting::GetProfessionName(player->profession);
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
        std::string formattedDistance = FormatDistance(entityContext.gameplayDistance, request.frameContext.settings);
        segments.push_back({ formattedDistance, ESPColors::DEFAULT_TEXT });
    }
    
    // Create and Style the TextElement
    TextElement element(segments, {0,0});
    TextStyle style = GetPlayerNameStyle(props.finalAlpha, props.fadedEntityColor, props.finalFontSize, request.frameContext.settings);
    style.useCustomTextColor = true;
    element.SetStyle(style);

    return element;
}

} // namespace kx
