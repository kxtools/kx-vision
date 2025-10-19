#include "TextRenderer.h"
#include "../../../libs/ImGui/imgui.h"
#include "../Utils/ESPConstants.h"

using namespace kx::RenderingLayout;

namespace kx {

void TextRenderer::Render(ImDrawList* drawList, const TextElement& element) {
    if (!drawList) return;
    
    // Critical: Check if ImGui context is still valid before any ImGui operations
    if (!ImGui::GetCurrentContext()) return;
    
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
            RenderBackground(drawList, linePos, textSize, style);
        }
        
        // Render border
        if (style.enableBorder) {
            RenderBorder(drawList, linePos, textSize, style);
        }
        
        // Render text
        RenderTextLine(drawList, line, linePos, style);
    }
}

void TextRenderer::RenderBatch(ImDrawList* drawList, const std::vector<TextElement>& elements) {
    for (const auto& element : elements) {
        Render(drawList, element);
    }
}

ImVec2 TextRenderer::CalculateSize(const TextElement& element) {
    const auto& lines = element.GetLines();
    if (lines.empty()) {
        return ImVec2(0, 0);
    }

    const auto& style = element.GetStyle();
    ImFont* font = ImGui::GetFont();

    float maxWidth = 0.0f;
    float totalHeight = 0.0f;

    for (const auto& line : lines) {
        float lineWidth = CalculateLineWidth(line, style.fontSize);
        if (lineWidth > maxWidth) {
            maxWidth = lineWidth;
        }
        float lineHeight = font->CalcTextSizeA(style.fontSize, FLT_MAX, 0.0f, " ").y;
        totalHeight += lineHeight;
    }

    if (lines.size() > 1) {
        totalHeight += element.GetLineSpacing() * (lines.size() - 1);
    }

    if (style.enableBackground) {
        maxWidth += style.backgroundPadding.x * 2;
        totalHeight += style.backgroundPadding.y * 2;
    }

    return ImVec2(maxWidth, totalHeight);
}

ImVec2 TextRenderer::CalculateLinePosition(const glm::vec2& anchor, float lineWidth, float totalHeight,
                                           int lineIndex, float lineHeight, TextAnchor positioning,
                                           const glm::vec2& customOffset, TextAlignment alignment,
                                           const TextStyle& style) {
    ImVec2 pos;
    
    // Calculate vertical position based on anchor
    switch (positioning) {
        case TextAnchor::Above:
            pos.y = anchor.y - totalHeight - TEXT_ANCHOR_GAP;
            break;
        case TextAnchor::Below:
            pos.y = anchor.y + TEXT_ANCHOR_GAP;
            break;
        case TextAnchor::Center:
            pos.y = anchor.y - totalHeight / 2.0f;
            break;
        case TextAnchor::Custom:
            pos.y = anchor.y + customOffset.y;
            break;
        case TextAnchor::AbsoluteTopLeft:
            pos.y = anchor.y;
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

void TextRenderer::RenderBackground(ImDrawList* drawList, const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style) {
    ImVec2 bgMin(textPos.x - style.backgroundPadding.x, textPos.y - style.backgroundPadding.y);
    ImVec2 bgMax(textPos.x + textSize.x + style.backgroundPadding.x, textPos.y + textSize.y + style.backgroundPadding.y);
    
    // Use more precise alpha calculation to avoid jitter
    float alphaf = style.backgroundAlpha * style.fadeAlpha * 255.0f;
    unsigned int bgAlpha = static_cast<unsigned int>(alphaf + 0.5f); // Round instead of truncate
    bgAlpha = (bgAlpha > 255) ? 255 : bgAlpha; // Clamp
    ImU32 bgColor = IM_COL32(0, 0, 0, bgAlpha);
    
    drawList->AddRectFilled(bgMin, bgMax, bgColor, style.backgroundRounding);
}

void TextRenderer::RenderBorder(ImDrawList* drawList, const ImVec2& textPos, const ImVec2& textSize, const TextStyle& style) {
    ImVec2 borderMin(textPos.x - style.backgroundPadding.x, textPos.y - style.backgroundPadding.y);
    ImVec2 borderMax(textPos.x + textSize.x + style.backgroundPadding.x, textPos.y + textSize.y + style.backgroundPadding.y);
    
    ImU32 borderColor = ApplyFade(style.borderColor, style.fadeAlpha);
    
    drawList->AddRect(borderMin, borderMax, borderColor, style.backgroundRounding, 0, style.borderThickness);
}

void TextRenderer::RenderTextLine(ImDrawList* drawList, const std::vector<TextSegment>& segments, const ImVec2& basePos, const TextStyle& style) {
    // Critical: Check if ImGui context is still valid before any ImGui operations
    if (!ImGui::GetCurrentContext()) return;
    
    ImFont* font = ImGui::GetFont();
    ImVec2 currentPos = basePos;
    
    for (const auto& segment : segments) {
        if (segment.text.empty()) continue;
        
        // Calculate segment size with proper scaling
        ImVec2 segmentSize = font->CalcTextSizeA(style.fontSize, FLT_MAX, 0.0f, segment.text.c_str());
        
        // Render shadow
        if (style.enableShadow) {
            float alphaf = style.shadowAlpha * style.fadeAlpha * 255.0f;
            unsigned int shadowAlpha = static_cast<unsigned int>(alphaf + 0.5f); // Round instead of truncate
            shadowAlpha = (shadowAlpha > 255) ? 255 : shadowAlpha; // Clamp
            ImVec2 shadowPos(currentPos.x + style.shadowOffset.x, currentPos.y + style.shadowOffset.y);
            drawList->AddText(font, style.fontSize, shadowPos, IM_COL32(0, 0, 0, shadowAlpha), segment.text.c_str());
        }
        
        // Render main text
        ImU32 textColor = style.useCustomTextColor ? segment.color : style.textColor;
        textColor = ApplyFade(textColor, style.fadeAlpha);
        drawList->AddText(font, style.fontSize, currentPos, textColor, segment.text.c_str());
        
        // Move to next segment position
        currentPos.x += segmentSize.x;
    }
}

ImU32 TextRenderer::ApplyFade(ImU32 color, float fadeAlpha) {
    // Extract original alpha component
    int a = (color >> IM_COL32_A_SHIFT) & 0xFF;
    
    // Use rounding for smoother alpha transitions
    float alphaf = static_cast<float>(a) * fadeAlpha;
    unsigned int newAlpha = static_cast<unsigned int>(alphaf + 0.5f);
    newAlpha = (newAlpha > 255) ? 255 : newAlpha; // Clamp

    // Preserve original RGB, only change alpha
    return (color & 0x00FFFFFF) | (static_cast<ImU32>(newAlpha) << IM_COL32_A_SHIFT);
}

float TextRenderer::CalculateLineWidth(const std::vector<TextSegment>& segments, float fontSize) {
    // Critical: Check if ImGui context is still valid before any ImGui operations
    if (!ImGui::GetCurrentContext()) return 0.0f;
    
    ImFont* font = ImGui::GetFont();
    float totalWidth = 0.0f;
    
    for (const auto& segment : segments) {
        ImVec2 segmentSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, segment.text.c_str());
        totalWidth += segmentSize.x;
    }
    
    return totalWidth;
}

} // namespace kx
