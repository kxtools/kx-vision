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
 * TextRenderer renderer(drawList);
 * 
 * TextElement nameText("Player Name", anchorPos, TextAnchor::Below);
 * nameText.SetFadeAlpha(0.8f);
 * renderer.Render(nameText);
 * 
 * TextElement details({"Level: 80", "HP: 100%"}, anchorPos, TextAnchor::Below);
 * renderer.Render(details);
 * ```
 */
class TextRenderer {
public:
    /**
     * @brief Construct a text renderer
     * @param drawList ImGui draw list to render to
     */
    explicit TextRenderer(ImDrawList* drawList);
    
    /**
     * @brief Render a single text element
     * @param element The text element to render
     */
    void Render(const TextElement& element);
    
    /**
     * @brief Render multiple text elements (batch operation)
     * @param elements Vector of text elements to render
     */
    void RenderBatch(const std::vector<TextElement>& elements);
    
private:
    /**
     * @brief Calculate the screen position for a line based on anchor and positioning
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
    ImVec2 CalculateLinePosition(const glm::vec2& anchor, float lineWidth, float totalHeight,
                                 int lineIndex, float lineHeight, TextAnchor positioning,
                                 const glm::vec2& customOffset, TextAlignment alignment,
                                 const TextStyle& style) const;
    
    /**
     * @brief Render background for a text line
     * @param textPos Position of the text
     * @param textSize Size of the text
     * @param style Style configuration
     */
    void RenderBackground(const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style);
    
    /**
     * @brief Render border for a text line
     * @param textPos Position of the text
     * @param textSize Size of the text
     * @param style Style configuration
     */
    void RenderBorder(const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style);
    
    /**
     * @brief Render a single line of text with multiple colored segments
     * @param segments Text segments to render
     * @param basePos Starting position
     * @param style Style configuration
     */
    void RenderTextLine(const std::vector<TextSegment>& segments, const ImVec2& basePos, const TextStyle& style);
    
    /**
     * @brief Apply fade alpha to a color
     * @param color Original color
     * @param fadeAlpha Fade multiplier (0.0 to 1.0)
     * @return Color with applied fade
     */
    ImU32 ApplyFade(ImU32 color, float fadeAlpha) const;
    
    /**
     * @brief Calculate total width of a line (sum of all segments)
     * @param segments Text segments
     * @param fontSize Font size
     * @return Total width in pixels
     */
    float CalculateLineWidth(const std::vector<TextSegment>& segments, float fontSize) const;
    
    ImDrawList* m_drawList;
};

} // namespace kx
