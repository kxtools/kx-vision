#pragma once

#include "TextElement.h"
#include <vector>

struct ImDrawList;

namespace kx {

/**
 * @brief High-level text rendering utility that handles all text drawing complexities
 * 
 * This class encapsulates:
 * - Position calculation based on anchor and positioning mode
 * - Background rendering with padding and rounding
 * - Shadow rendering
 * - Border rendering
 * - Multi-line layout with spacing
 * - Multi-colored text segments
 * - Distance-based fading
 * 
 * Usage:
 * ```cpp
 * TextElement nameText("Player Name", anchorPos, TextAnchor::Below);
 * nameText.SetFadeAlpha(0.8f);
 * TextRenderer::Render(drawList, nameText);
 * 
 * TextElement details({"Level: 80", "HP: 100%"}, anchorPos, TextAnchor::Below);
 * TextRenderer::Render(drawList, details);
 * ```
 */
class TextRenderer {
public:
    /**
     * @brief Render a single text element
     * @param drawList ImGui draw list to render to
     * @param element The text element to render
     */
    static void Render(ImDrawList* drawList, const TextElement& element);
    
    /**
     * @brief Render multiple text elements (batch operation)
     * @param drawList ImGui draw list to render to
     * @param elements Vector of text elements to render
     */
    static void RenderBatch(ImDrawList* drawList, const std::vector<TextElement>& elements);

    /**
     * @brief Calculate the total size of a text element, including padding
     * @param element The text element to measure
     * @return The total size (width, height) as an ImVec2
     */
    static ImVec2 CalculateSize(const TextElement& element);
    
private:
    /**
     * @brief Calculate the screen position for a line based on anchor and positioning
     * @param drawList ImGui draw list
     * @param anchor The anchor point
     * @param lineWidth Width of the text line
     * @param totalHeight Total height of all lines
     * @param lineIndex Which line (0-based)
     * @param positioning Positioning mode
     * @param customOffset Custom offset (used if positioning == Custom)
     * @param alignment Horizontal alignment
     * @param style Style configuration
     * @return Final screen position for the line
     */
    static ImVec2 CalculateLinePosition(const glm::vec2& anchor, float lineWidth, float totalHeight,
                                 int lineIndex, float lineHeight, TextAnchor positioning,
                                 const glm::vec2& customOffset, TextAlignment alignment,
                                 const TextStyle& style);
    
    /**
     * @brief Render background for a text line
     * @param drawList ImGui draw list to render to
     * @param textPos Position of the text
     * @param textSize Size of the text
     * @param style Style configuration
     */
    static void RenderBackground(ImDrawList* drawList, const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style);
    
    /**
     * @brief Render border for a text line
     * @param drawList ImGui draw list to render to
     * @param textPos Position of the text
     * @param textSize Size of the text
     * @param style Style configuration
     */
    static void RenderBorder(ImDrawList* drawList, const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style);
    
    /**
     * @brief Render a single line of text with multiple colored segments
     * @param drawList ImGui draw list to render to
     * @param segments Text segments to render
     * @param basePos Starting position
     * @param style Style configuration
     */
    static void RenderTextLine(ImDrawList* drawList, const std::vector<TextSegment>& segments, const ImVec2& basePos, const TextStyle& style);
    
    /**
     * @brief Apply fade alpha to a color
     * @param color Original color
     * @param fadeAlpha Fade multiplier (0.0 to 1.0)
     * @return Color with applied fade
     */
    static ImU32 ApplyFade(ImU32 color, float fadeAlpha);
    
    /**
     * @brief Calculate total width of a line (sum of all segments)
     * @param segments Text segments
     * @param fontSize Font size
     * @return Total width in pixels
     */
    static float CalculateLineWidth(const std::vector<TextSegment>& segments, float fontSize);
};

} // namespace kx
