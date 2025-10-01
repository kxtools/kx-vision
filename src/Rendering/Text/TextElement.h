#pragma once

#include <string>
#include <vector>
#include <glm.hpp>
#include "../../../libs/ImGui/imgui.h"

namespace kx {

/**
 * @brief Positioning mode for text elements relative to an anchor point
 */
enum class TextAnchor {
    Above,      // Above the anchor point (e.g., distance text above box)
    Below,      // Below the anchor point (e.g., details below box)
    Center,     // Centered on the anchor point
    Custom      // Use custom offset from anchor
};

/**
 * @brief Horizontal alignment for text
 */
enum class TextAlignment {
    Left,
    Center,
    Right
};

/**
 * @brief Style configuration for text rendering
 */
struct TextStyle {
    float fontSize = 13.0f;
    
    // Text colors
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    bool useCustomTextColor = false;  // If false, uses textColor; if true, each segment can have its own color
    
    // Shadow
    bool enableShadow = true;
    ImVec2 shadowOffset = ImVec2(1.0f, 1.0f);
    float shadowAlpha = 128.0f / 255.0f;  // 0.0 to 1.0
    
    // Background
    bool enableBackground = true;
    ImVec2 backgroundPadding = ImVec2(4.0f, 2.0f);
    float backgroundAlpha = 180.0f / 255.0f;  // 0.0 to 1.0
    float backgroundRounding = 3.0f;
    
    // Border
    bool enableBorder = false;
    ImU32 borderColor = IM_COL32(255, 255, 255, 128);
    float borderThickness = 1.0f;
    
    // Distance fading
    float fadeAlpha = 1.0f;  // Overall fade multiplier (0.0 to 1.0)
};

/**
 * @brief A single colored text segment (for multi-colored text)
 */
struct TextSegment {
    std::string text;
    ImU32 color = IM_COL32(255, 255, 255, 255);
    
    TextSegment(const std::string& txt, ImU32 col = IM_COL32(255, 255, 255, 255))
        : text(txt), color(col) {}
};

/**
 * @brief A text element that can be rendered
 * 
 * Supports:
 * - Single-line or multi-line text
 * - Multi-colored segments on a single line
 * - Custom positioning relative to anchor
 * - Styling (shadow, background, border)
 * - Distance-based fading
 */
class TextElement {
public:
    /**
     * @brief Simple text element with single color
     */
    TextElement(const std::string& text, const glm::vec2& anchor, TextAnchor positioning = TextAnchor::Below)
        : m_anchor(anchor), m_positioning(positioning), m_alignment(TextAlignment::Center) {
        m_lines.push_back({TextSegment(text)});
    }
    
    /**
     * @brief Text element with custom offset
     */
    TextElement(const std::string& text, const glm::vec2& anchor, const glm::vec2& customOffset)
        : m_anchor(anchor), m_positioning(TextAnchor::Custom), m_customOffset(customOffset), m_alignment(TextAlignment::Center) {
        m_lines.push_back({TextSegment(text)});
    }
    
    /**
     * @brief Multi-line text element
     */
    TextElement(const std::vector<std::string>& lines, const glm::vec2& anchor, TextAnchor positioning = TextAnchor::Below)
        : m_anchor(anchor), m_positioning(positioning), m_alignment(TextAlignment::Center) {
        for (const auto& line : lines) {
            m_lines.push_back({TextSegment(line)});
        }
    }
    
    /**
     * @brief Multi-colored single-line text element
     */
    TextElement(const std::vector<TextSegment>& segments, const glm::vec2& anchor, TextAnchor positioning = TextAnchor::Below)
        : m_anchor(anchor), m_positioning(positioning), m_alignment(TextAlignment::Center) {
        m_lines.push_back(segments);
    }
    
    /**
     * @brief Multi-line, multi-colored text element
     */
    TextElement(const std::vector<std::vector<TextSegment>>& lines, const glm::vec2& anchor, TextAnchor positioning = TextAnchor::Below)
        : m_lines(lines), m_anchor(anchor), m_positioning(positioning), m_alignment(TextAlignment::Center) {}
    
    // Setters for fluent API
    TextElement& SetStyle(const TextStyle& style) { m_style = style; return *this; }
    TextElement& SetAlignment(TextAlignment alignment) { m_alignment = alignment; return *this; }
    TextElement& SetFadeAlpha(float alpha) { m_style.fadeAlpha = alpha; return *this; }
    TextElement& SetLineSpacing(float spacing) { m_lineSpacing = spacing; return *this; }
    
    // Getters
    const std::vector<std::vector<TextSegment>>& GetLines() const { return m_lines; }
    const glm::vec2& GetAnchor() const { return m_anchor; }
    TextAnchor GetPositioning() const { return m_positioning; }
    const glm::vec2& GetCustomOffset() const { return m_customOffset; }
    TextAlignment GetAlignment() const { return m_alignment; }
    const TextStyle& GetStyle() const { return m_style; }
    float GetLineSpacing() const { return m_lineSpacing; }
    
private:
    std::vector<std::vector<TextSegment>> m_lines;  // Each line can have multiple colored segments
    glm::vec2 m_anchor;                             // Reference point for positioning
    TextAnchor m_positioning;                       // How to position relative to anchor
    glm::vec2 m_customOffset = {0.0f, 0.0f};       // Used when positioning == Custom
    TextAlignment m_alignment = TextAlignment::Center;
    TextStyle m_style;
    float m_lineSpacing = 2.0f;                     // Spacing between lines in pixels
};

} // namespace kx
