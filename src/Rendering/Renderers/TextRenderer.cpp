#include "TextRenderer.h"
#include "../../Core/AppState.h"
#include "../Shared/LayoutConstants.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

    static inline ImU32 FadeColor(ImU32 color, float opacity) {
        int a = (color >> 24) & 0xFF;
        int newA = static_cast<int>(a * opacity);
        newA = (newA > 255) ? 255 : newA;
        return (color & 0x00FFFFFF) | (newA << 24);
    }

    float TextRenderer::DrawCentered(ImDrawList* dl, const glm::vec2& pos, std::string_view text, const FastTextStyle& style) {
        if (text.empty()) return 0.0f;

        ImFont* font = ImGui::GetFont();
        
        const char* text_begin = text.data();
        const char* text_end = text.data() + text.size();

        ImVec2 textSize = font->CalcTextSizeA(style.fontSize, FLT_MAX, 0.0f, text_begin, text_end);
        
        float x = pos.x - (textSize.x * 0.5f);
        float y = pos.y;

        float globalOpacity = AppState::Get().GetSettings().appearance.globalOpacity;
        float combinedOpacity = style.fadeAlpha * globalOpacity;

        if (style.background) {
            float paddingX = RenderingLayout::TEXT_DEFAULT_BG_PADDING_X;
            float paddingY = RenderingLayout::TEXT_DEFAULT_BG_PADDING_Y;
            ImU32 bgCol = IM_COL32(0, 0, 0, static_cast<int>(96 * combinedOpacity));
            
            dl->AddRectFilled(
                ImVec2(x - paddingX, y - paddingY),
                ImVec2(x + textSize.x + paddingX, y + textSize.y + paddingY),
                bgCol,
                RenderingLayout::TEXT_DEFAULT_BG_ROUNDING
            );
        }

        if (style.shadow) {
            ImU32 shadowCol = IM_COL32(0, 0, 0, static_cast<int>(255 * combinedOpacity));
            dl->AddText(font, style.fontSize, ImVec2(x + 1, y + 1), shadowCol, text_begin, text_end);
        }

        ImU32 textCol = FadeColor(style.color, combinedOpacity);
        dl->AddText(font, style.fontSize, ImVec2(x, y), textCol, text_begin, text_end);

        return textSize.y;
    }

    float TextRenderer::DrawMultiColored(ImDrawList* dl, const glm::vec2& pos, 
                                         int count, const std::string_view* texts, 
                                         const ImU32* colors, const FastTextStyle& style) {
        if (count <= 0) return 0.0f;

        ImFont* font = ImGui::GetFont();
        
        float totalWidth = 0.0f;
        float maxHeight = 0.0f;
        
        float segmentWidths[16]; 
        int safeCount = (count > 16) ? 16 : count;

        for (int i = 0; i < safeCount; i++) {
            const char* text_begin = texts[i].data();
            const char* text_end = texts[i].data() + texts[i].size();
            ImVec2 size = font->CalcTextSizeA(style.fontSize, FLT_MAX, 0.0f, text_begin, text_end);
            segmentWidths[i] = size.x;
            totalWidth += size.x;
            if (size.y > maxHeight) maxHeight = size.y;
        }

        float x = pos.x - (totalWidth * 0.5f);
        float y = pos.y;
        float globalOpacity = AppState::Get().GetSettings().appearance.globalOpacity;
        float combinedOpacity = style.fadeAlpha * globalOpacity;

        if (style.background) {
            float paddingX = RenderingLayout::TEXT_DEFAULT_BG_PADDING_X;
            float paddingY = RenderingLayout::TEXT_DEFAULT_BG_PADDING_Y;
            ImU32 bgCol = IM_COL32(0, 0, 0, static_cast<int>(96 * combinedOpacity));
            
            dl->AddRectFilled(
                ImVec2(x - paddingX, y - paddingY),
                ImVec2(x + totalWidth + paddingX, y + maxHeight + paddingY),
                bgCol,
                RenderingLayout::TEXT_DEFAULT_BG_ROUNDING
            );
        }

        float currentX = x;
        for (int i = 0; i < safeCount; i++) {
            const char* text_begin = texts[i].data();
            const char* text_end = texts[i].data() + texts[i].size();
            
            if (style.shadow) {
                ImU32 shadowCol = IM_COL32(0, 0, 0, static_cast<int>(255 * combinedOpacity));
                dl->AddText(font, style.fontSize, ImVec2(currentX + 1, y + 1), shadowCol, text_begin, text_end);
            }

            ImU32 textCol = FadeColor(colors[i], combinedOpacity);
            dl->AddText(font, style.fontSize, ImVec2(currentX, y), textCol, text_begin, text_end);

            currentX += segmentWidths[i];
        }

        return maxHeight;
    }
}
