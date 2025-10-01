#include "TextRenderer.h"
#include "../../../libs/ImGui/imgui.h"
#include "../Utils/ESPConstants.h"

using namespace kx::RenderingLayout;

namespace kx {

TextRenderer::TextRenderer(ImDrawList* drawList)
    : m_drawList(drawList) {}

void TextRenderer::Render(const TextElement& element) {
    if (!m_drawList) return;
    
    const auto& lines = element.GetLines();
    if (lines.empty()) return;
    
    const auto& style = element.GetStyle();
    ImFont* font = ImGui::GetFont();
    
    // Calculate dimensions
    float totalHeight = 0.0f;
    std::vector<float> lineWidths;
    std::vector<float> lineHeights;
    
    for (const auto& line : lines) {
        float lineWidth = CalculateLineWidth(line, style.fontSize);
        float lineHeight = font->CalcTextSizeA(style.fontSize, FLT_MAX, 0.0f, " ").y;
        
        lineWidths.push_back(lineWidth);
        lineHeights.push_back(lineHeight);
        totalHeight += lineHeight;
    }
    
    // Add spacing between lines
    if (lines.size() > 1) {
        totalHeight += element.GetLineSpacing() * (lines.size() - 1);
    }
    
    // Render each line
    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        float lineWidth = lineWidths[i];
        float lineHeight = lineHeights[i];
        
        // Calculate position for this line
        ImVec2 linePos = CalculateLinePosition(
            element.GetAnchor(),
            lineWidth,
            totalHeight,
            static_cast<int>(i),
            lineHeight,
            element.GetPositioning(),
            element.GetCustomOffset(),
            element.GetAlignment(),
            style
        );
        
        ImVec2 textSize(lineWidth, lineHeight);
        
        // Render background
        if (style.enableBackground) {
            RenderBackground(linePos, textSize, style);
        }
        
        // Render border
        if (style.enableBorder) {
            RenderBorder(linePos, textSize, style);
        }
        
        // Render text
        RenderTextLine(line, linePos, style);
    }
}

void TextRenderer::RenderBatch(const std::vector<TextElement>& elements) {
    for (const auto& element : elements) {
        Render(element);
    }
}

ImVec2 TextRenderer::CalculateLinePosition(const glm::vec2& anchor, float lineWidth, float totalHeight,
                                           int lineIndex, float lineHeight, TextAnchor positioning,
                                           const glm::vec2& customOffset, TextAlignment alignment,
                                           const TextStyle& style) const {
    ImVec2 pos;
    
    // Calculate vertical position based on anchor
    switch (positioning) {
        case TextAnchor::Above:
            pos.y = anchor.y - totalHeight - RenderingLayout::TEXT_ANCHOR_GAP;
            break;
        case TextAnchor::Below:
            pos.y = anchor.y + RenderingLayout::TEXT_ANCHOR_GAP;
            break;
        case TextAnchor::Center:
            pos.y = anchor.y - totalHeight / 2.0f;
            break;
        case TextAnchor::Custom:
            pos.y = anchor.y + customOffset.y;
            break;
    }
    
    // Add offset for this specific line
    ImFont* font = ImGui::GetFont();
    float lineSpacing = font->CalcTextSizeA(style.fontSize, FLT_MAX, 0.0f, " ").y + TEXT_LINE_SPACING_EXTRA;
    pos.y += lineIndex * lineSpacing;
    
    // Calculate horizontal position based on alignment
    switch (alignment) {
        case TextAlignment::Left:
            pos.x = anchor.x;
            break;
        case TextAlignment::Center:
            pos.x = anchor.x - lineWidth / 2.0f;
            break;
        case TextAlignment::Right:
            pos.x = anchor.x - lineWidth;
            break;
    }
    
    // Apply custom offset for horizontal if Custom positioning
    if (positioning == TextAnchor::Custom) {
        pos.x = anchor.x + customOffset.x;
        if (alignment == TextAlignment::Center) {
            pos.x -= lineWidth / 2.0f;
        } else if (alignment == TextAlignment::Right) {
            pos.x -= lineWidth;
        }
    }
    
    return pos;
}

void TextRenderer::RenderBackground(const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style) {
    ImVec2 bgMin(textPos.x - style.backgroundPadding.x, textPos.y - style.backgroundPadding.y);
    ImVec2 bgMax(textPos.x + textSize.x + style.backgroundPadding.x, textPos.y + textSize.y + style.backgroundPadding.y);
    
    unsigned int bgAlpha = static_cast<unsigned int>(style.backgroundAlpha * style.fadeAlpha * 255.0f);
    ImU32 bgColor = IM_COL32(0, 0, 0, bgAlpha);
    
    m_drawList->AddRectFilled(bgMin, bgMax, bgColor, style.backgroundRounding);
}

void TextRenderer::RenderBorder(const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style) {
    ImVec2 borderMin(textPos.x - style.backgroundPadding.x, textPos.y - style.backgroundPadding.y);
    ImVec2 borderMax(textPos.x + textSize.x + style.backgroundPadding.x, textPos.y + textSize.y + style.backgroundPadding.y);
    
    ImU32 borderColor = ApplyFade(style.borderColor, style.fadeAlpha);
    
    m_drawList->AddRect(borderMin, borderMax, borderColor, style.backgroundRounding, 0, style.borderThickness);
}

void TextRenderer::RenderTextLine(const std::vector<TextSegment>& segments, const ImVec2& basePos, const TextStyle& style) {
    ImFont* font = ImGui::GetFont();
    ImVec2 currentPos = basePos;
    
    for (const auto& segment : segments) {
        if (segment.text.empty()) continue;
        
        ImVec2 segmentSize = font->CalcTextSizeA(style.fontSize, FLT_MAX, 0.0f, segment.text.c_str());
        
        // Render shadow
        if (style.enableShadow) {
            unsigned int shadowAlpha = static_cast<unsigned int>(style.shadowAlpha * style.fadeAlpha * 255.0f);
            ImVec2 shadowPos(currentPos.x + style.shadowOffset.x, currentPos.y + style.shadowOffset.y);
            m_drawList->AddText(font, style.fontSize, shadowPos, IM_COL32(0, 0, 0, shadowAlpha), segment.text.c_str());
        }
        
        // Render main text
        ImU32 textColor = style.useCustomTextColor ? segment.color : style.textColor;
        textColor = ApplyFade(textColor, style.fadeAlpha);
        m_drawList->AddText(font, style.fontSize, currentPos, textColor, segment.text.c_str());
        
        // Move to next segment position
        currentPos.x += segmentSize.x;
    }
}

ImU32 TextRenderer::ApplyFade(ImU32 color, float fadeAlpha) const {
    int r = (color >> 0) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = (color >> 16) & 0xFF;
    int a = (color >> 24) & 0xFF;
    
    unsigned int newAlpha = static_cast<unsigned int>(a * fadeAlpha);
    return IM_COL32(r, g, b, newAlpha);
}

float TextRenderer::CalculateLineWidth(const std::vector<TextSegment>& segments, float fontSize) const {
    ImFont* font = ImGui::GetFont();
    float totalWidth = 0.0f;
    
    for (const auto& segment : segments) {
        ImVec2 segmentSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, segment.text.c_str());
        totalWidth += segmentSize.x;
    }
    
    return totalWidth;
}

} // namespace kx
