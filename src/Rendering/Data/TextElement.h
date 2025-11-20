#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <utility>
#include <glm.hpp>
#include "../../../libs/ImGui/imgui.h"
#include "Shared/LayoutConstants.h"

namespace kx {

/**
 * @brief Positioning mode for text elements relative to an anchor point
 */
enum class TextAnchor {
    Above,      // Above the anchor point (e.g., distance text above box)
    Below,      // Below the anchor point (e.g., details below box)
    Center,     // Centered on the anchor point
    Custom,     // Use custom offset from anchor
    AbsoluteTopLeft // Absolute top-left position
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
    float fontSize = RenderingLayout::TEXT_DEFAULT_FONT_SIZE;
    
    // Text colors
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    bool useCustomTextColor = false;  // If false, uses textColor; if true, each segment can have its own color
    
    // Shadow
    bool enableShadow = true;
    ImVec2 shadowOffset = ImVec2(RenderingLayout::TEXT_DEFAULT_SHADOW_OFFSET_X, RenderingLayout::TEXT_DEFAULT_SHADOW_OFFSET_Y);
    float shadowAlpha = RenderingLayout::TEXT_DEFAULT_SHADOW_ALPHA / 255.0f;  // 0.0 to 1.0
    
    // Background
    bool enableBackground = true;
    ImVec2 backgroundPadding = ImVec2(RenderingLayout::TEXT_DEFAULT_BG_PADDING_X, RenderingLayout::TEXT_DEFAULT_BG_PADDING_Y);
    float backgroundAlpha = RenderingLayout::TEXT_DEFAULT_BG_ALPHA / 255.0f;  // 0.0 to 1.0
    float backgroundRounding = RenderingLayout::TEXT_DEFAULT_BG_ROUNDING;
    
    // Border
    bool enableBorder = false;
    ImU32 borderColor = IM_COL32(255, 255, 255, 128);
    float borderThickness = RenderingLayout::TEXT_DEFAULT_BORDER_THICKNESS;
    
    // Distance fading
    float fadeAlpha = 1.0f;  // Overall fade multiplier (0.0 to 1.0)
};

/**
 * @brief A single colored text segment (for multi-colored text)
 */
struct TextSegment {
    std::string text;
    ImU32 color = IM_COL32(255, 255, 255, 255);
    
    TextSegment() = default;
    TextSegment(std::string txt, ImU32 col = IM_COL32(255, 255, 255, 255))
        : text(std::move(txt)), color(col) {}
    TextSegment(std::string_view txt, ImU32 col = IM_COL32(255, 255, 255, 255))
        : text(txt), color(col) {}
    TextSegment(const char* txt, ImU32 col = IM_COL32(255, 255, 255, 255))
        : text(txt ? txt : ""), color(col) {}
};

/**
 * @brief Helper struct to track where lines start and end in the flat buffer
 */
struct LineSpan {
    uint16_t startIdx;
    uint16_t count;
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
        m_segments.emplace_back(text);
        m_lines.push_back({ 0, 1 });
    }
    
    /**
     * @brief Text element with custom offset
     */
    TextElement(const std::string& text, const glm::vec2& anchor, const glm::vec2& customOffset)
        : m_anchor(anchor), m_positioning(TextAnchor::Custom), m_customOffset(customOffset), m_alignment(TextAlignment::Center) {
        m_segments.emplace_back(text);
        m_lines.push_back({ 0, 1 });
    }
    
    /**
     * @brief Multi-line text element
     */
    TextElement(const std::vector<std::string>& lines, const glm::vec2& anchor, TextAnchor positioning = TextAnchor::Below)
        : m_anchor(anchor), m_positioning(positioning), m_alignment(TextAlignment::Center) {
        m_segments.reserve(lines.size());
        m_lines.reserve(lines.size());
        
        for (const auto& line : lines) {
            uint16_t start = static_cast<uint16_t>(m_segments.size());
            m_segments.emplace_back(line);
            m_lines.push_back({ start, 1 });
        }
    }
    
    /**
     * @brief Multi-colored single-line text element
     */
    TextElement(const std::vector<TextSegment>& segments, const glm::vec2& anchor, TextAnchor positioning = TextAnchor::Below)
        : m_anchor(anchor), m_positioning(positioning), m_alignment(TextAlignment::Center) {
        m_segments = segments;
        m_lines.push_back({ 0, static_cast<uint16_t>(segments.size()) });
    }
    
    /**
     * @brief Multi-line, multi-colored text element
     */
    TextElement(const std::vector<std::vector<TextSegment>>& lines, const glm::vec2& anchor, TextAnchor positioning = TextAnchor::Below)
        : m_anchor(anchor), m_positioning(positioning), m_alignment(TextAlignment::Center) {
        size_t totalSegments = 0;
        for (const auto& line : lines) totalSegments += line.size();
        
        m_segments.reserve(totalSegments);
        m_lines.reserve(lines.size());

        for (const auto& line : lines) {
            uint16_t start = static_cast<uint16_t>(m_segments.size());
            m_segments.insert(m_segments.end(), line.begin(), line.end());
            m_lines.push_back({ start, static_cast<uint16_t>(line.size()) });
        }
    }
    
    // Setters for fluent API
    TextElement& SetStyle(const TextStyle& style) { m_style = style; return *this; }
    TextElement& SetAlignment(TextAlignment alignment) { m_alignment = alignment; return *this; }
    TextElement& SetFadeAlpha(float alpha) { m_style.fadeAlpha = alpha; return *this; }
    TextElement& SetLineSpacing(float spacing) { m_lineSpacing = spacing; return *this; }
    TextElement& SetAnchor(const glm::vec2& anchor) { m_anchor = anchor; return *this; }
    TextElement& SetPositioning(TextAnchor positioning) { m_positioning = positioning; return *this; }
    
    // Getters
    const std::vector<TextSegment>& GetSegments() const { return m_segments; }
    const std::vector<LineSpan>& GetLines() const { return m_lines; }
    const glm::vec2& GetAnchor() const { return m_anchor; }
    TextAnchor GetPositioning() const { return m_positioning; }
    const glm::vec2& GetCustomOffset() const { return m_customOffset; }
    TextAlignment GetAlignment() const { return m_alignment; }
    const TextStyle& GetStyle() const { return m_style; }
    float GetLineSpacing() const { return m_lineSpacing; }
    
private:
    std::vector<TextSegment> m_segments;
    std::vector<LineSpan> m_lines;
    glm::vec2 m_anchor;                             // Reference point for positioning
    TextAnchor m_positioning;                       // How to position relative to anchor
    glm::vec2 m_customOffset = {0.0f, 0.0f};       // Used when positioning == Custom
    TextAlignment m_alignment = TextAlignment::Center;
    TextStyle m_style;
    float m_lineSpacing = 2.0f;                     // Spacing between lines in pixels
};

} // namespace kx
