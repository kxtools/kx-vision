#include "ESPTextRenderer.h"
#include "ESPConstants.h"
#include "ESPStyling.h"
#include "../../libs/ImGui/imgui.h"

namespace kx {

void ESPTextRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos,
                                      const std::string& playerName, unsigned int entityColor, float fontSize) {
    if (playerName.empty()) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    ImFont* font = ImGui::GetFont();
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, playerName.c_str());

    // Position name just below the feet position (below health bar area)
    const float nameOffset = RenderingLayout::PLAYER_NAME_Y_OFFSET;
    ImVec2 textPos(feetPos.x - textSize.x / 2, feetPos.y + nameOffset);

    // Extract RGB from entity color for a natural look
    int r = (entityColor >> 16) & 0xFF;
    int g = (entityColor >> 8) & 0xFF;
    int b = entityColor & 0xFF;

    // Subtle background with rounded corners (like game UI) and distance fade
    ImVec2 bgMin(textPos.x - RenderingLayout::PLAYER_NAME_BG_PADDING_X, textPos.y - RenderingLayout::PLAYER_NAME_BG_PADDING_Y);
    ImVec2 bgMax(textPos.x + textSize.x + RenderingLayout::PLAYER_NAME_BG_PADDING_X, textPos.y + textSize.y + RenderingLayout::PLAYER_NAME_BG_PADDING_Y);
    unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_BG_ALPHA * fadeAlpha);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::PLAYER_NAME_BG_ROUNDING);

    // Subtle border using entity color with distance fade
    unsigned int borderAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_BORDER_ALPHA * fadeAlpha);
    drawList->AddRect(bgMin, bgMax, IM_COL32(r, g, b, borderAlpha), RenderingLayout::PLAYER_NAME_BG_ROUNDING, 0, RenderingLayout::PLAYER_NAME_BORDER_THICKNESS);

    // Player name text in a clean, readable color with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_SHADOW_ALPHA * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_TEXT_ALPHA * fadeAlpha);
    drawList->AddText(font, fontSize, ImVec2(textPos.x + RenderingLayout::TEXT_SHADOW_OFFSET, textPos.y + RenderingLayout::TEXT_SHADOW_OFFSET), IM_COL32(0, 0, 0, shadowAlpha), playerName.c_str()); // Shadow
    drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, textAlpha), playerName.c_str()); // Main text
}

void ESPTextRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin,
                                        float distance, float fadeAlpha, float fontSize) {
    char distText[32];
    snprintf(distText, sizeof(distText), "%.1fm", distance);

    ImFont* font = ImGui::GetFont();
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, distText);
    ImVec2 textPos(center.x - textSize.x / 2, boxMin.y - textSize.y - RenderingLayout::DISTANCE_TEXT_Y_OFFSET);

    // Background with distance fade
    unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::DISTANCE_TEXT_BG_ALPHA * fadeAlpha);
    drawList->AddRectFilled(ImVec2(textPos.x - RenderingLayout::DISTANCE_TEXT_BG_PADDING_X, textPos.y - RenderingLayout::DISTANCE_TEXT_BG_PADDING_Y),
                           ImVec2(textPos.x + textSize.x + RenderingLayout::DISTANCE_TEXT_BG_PADDING_X, textPos.y + textSize.y + RenderingLayout::DISTANCE_TEXT_BG_PADDING_Y),
                           IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::DISTANCE_TEXT_BG_ROUNDING);

    // Text with shadow and distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::DISTANCE_TEXT_SHADOW_ALPHA * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(RenderingLayout::DISTANCE_TEXT_TEXT_ALPHA * fadeAlpha);
    drawList->AddText(font, fontSize, ImVec2(textPos.x + RenderingLayout::TEXT_SHADOW_OFFSET, textPos.y + RenderingLayout::TEXT_SHADOW_OFFSET), IM_COL32(0, 0, 0, shadowAlpha), distText);
    drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, textAlpha), distText);
}

void ESPTextRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax,
                                       const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize) {
    if (details.empty()) return;

    float textY = boxMax.y + RenderingLayout::DETAILS_TEXT_Y_OFFSET;
    ImFont* font = ImGui::GetFont();

    for (const auto& detail : details) {
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, detail.text.c_str());
        ImVec2 textPos(center.x - textSize.x / 2, textY);

        // Background with distance fade
        unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::DETAILS_TEXT_BG_ALPHA * fadeAlpha);
        drawList->AddRectFilled(ImVec2(textPos.x - RenderingLayout::DETAILS_TEXT_BG_PADDING_X, textPos.y - RenderingLayout::DETAILS_TEXT_BG_PADDING_Y),
                               ImVec2(textPos.x + textSize.x + RenderingLayout::DETAILS_TEXT_BG_PADDING_X, textPos.y + textSize.y + RenderingLayout::DETAILS_TEXT_BG_PADDING_Y),
                               IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::DETAILS_TEXT_BG_ROUNDING);

        // Text with shadow and distance fade
        unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::DETAILS_TEXT_SHADOW_ALPHA * fadeAlpha);

        // Extract the original alpha from the detail color and combine it with the fade alpha
        int originalAlpha = (detail.color >> 24) & 0xFF;
        unsigned int textAlpha = static_cast<unsigned int>(originalAlpha * fadeAlpha);

        // Re-create the final text color with the new combined alpha
        ImU32 finalTextColor = (detail.color & 0x00FFFFFF) | (textAlpha << 24);

        drawList->AddText(font, fontSize, ImVec2(textPos.x + RenderingLayout::TEXT_SHADOW_OFFSET, textPos.y + RenderingLayout::TEXT_SHADOW_OFFSET), IM_COL32(0, 0, 0, shadowAlpha), detail.text.c_str());
        drawList->AddText(font, fontSize, textPos, finalTextColor, detail.text.c_str());

        textY += textSize.y + RenderingLayout::DETAILS_TEXT_LINE_SPACING;
    }
}

void ESPTextRenderer::RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
                                       const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize) {
    if (summary.empty()) return;

    ImFont* font = ImGui::GetFont();

    // --- Part 1: Calculate total width for centering ---
    float totalWidth = 0.0f;
    const std::string prefix = "Stats: ";
    totalWidth += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, prefix.c_str()).x;

    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        std::string segment = std::to_string(info.count) + "x " + info.statName;
        totalWidth += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, segment.c_str()).x;
        if (i < summary.size() - 1) {
            totalWidth += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, ", ").x;
        }
    }

    // --- Part 2: Render the multi-colored line ---
    const float summaryOffset = RenderingLayout::SUMMARY_Y_OFFSET;
    ImVec2 currentPos(feetPos.x - totalWidth / 2.0f, feetPos.y + summaryOffset);

    // Fade alphas
    unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::SUMMARY_BG_ALPHA * fadeAlpha);
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::SUMMARY_SHADOW_ALPHA * fadeAlpha);
    unsigned int defaultTextAlpha = static_cast<unsigned int>(RenderingLayout::SUMMARY_TEXT_ALPHA * fadeAlpha);

    // Background
    ImVec2 bgMin(currentPos.x - RenderingLayout::SUMMARY_BG_PADDING_X, currentPos.y - RenderingLayout::SUMMARY_BG_PADDING_Y);
    ImVec2 bgMax(currentPos.x + totalWidth + RenderingLayout::SUMMARY_BG_PADDING_X, currentPos.y + font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, " ").y + RenderingLayout::SUMMARY_BG_PADDING_Y);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::SUMMARY_BG_ROUNDING);

    // Render "Stats: " prefix in default color
    drawList->AddText(font, fontSize, ImVec2(currentPos.x + RenderingLayout::TEXT_SHADOW_OFFSET, currentPos.y + RenderingLayout::TEXT_SHADOW_OFFSET), IM_COL32(0, 0, 0, shadowAlpha), prefix.c_str());
    drawList->AddText(font, fontSize, currentPos, IM_COL32(200, 210, 255, defaultTextAlpha), prefix.c_str());
    currentPos.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, prefix.c_str()).x;

    // Render each colored segment
    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        std::string segment = std::to_string(info.count) + "x " + info.statName;
        ImU32 rarityColor = ESPHelpers::GetRarityColor(info.highestRarity);

        // Apply fade to the rarity color
        int r = (rarityColor >> 0) & 0xFF;
        int g = (rarityColor >> 8) & 0xFF;
        int b = (rarityColor >> 16) & 0xFF;
        rarityColor = IM_COL32(r, g, b, defaultTextAlpha);

        drawList->AddText(font, fontSize, ImVec2(currentPos.x + 1, currentPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), segment.c_str());
        drawList->AddText(font, fontSize, currentPos, rarityColor, segment.c_str());
        currentPos.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, segment.c_str()).x;

        // Render separator
        if (i < summary.size() - 1) {
            const std::string separator = ", ";
            drawList->AddText(font, fontSize, ImVec2(currentPos.x + RenderingLayout::TEXT_SHADOW_OFFSET, currentPos.y + RenderingLayout::TEXT_SHADOW_OFFSET), IM_COL32(0, 0, 0, shadowAlpha), separator.c_str());
            drawList->AddText(font, fontSize, currentPos, IM_COL32(200, 210, 255, defaultTextAlpha), separator.c_str());
            currentPos.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, separator.c_str()).x;
        }
    }
}

void ESPTextRenderer::RenderDominantStats(ImDrawList* drawList, const glm::vec2& feetPos,
                                         const std::vector<DominantStat>& stats, float fadeAlpha, float fontSize) {
    if (stats.empty()) return;

    ImFont* font = ImGui::GetFont();

    // 1. Build the display string
    std::string summaryText = "[";
    for (size_t i = 0; i < stats.size(); ++i) {
        summaryText += stats[i].name;
        if (i < stats.size() - 1) {
            summaryText += " | ";
        }
    }
    summaryText += "]";

    // 2. Calculate width and position
    float totalWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, summaryText.c_str()).x;
    const float summaryOffset = RenderingLayout::SUMMARY_Y_OFFSET;
    ImVec2 textPos(feetPos.x - totalWidth / 2.0f, feetPos.y + summaryOffset);

    // 3. Render
    unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::SUMMARY_BG_ALPHA * fadeAlpha);
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::SUMMARY_SHADOW_ALPHA * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(RenderingLayout::SUMMARY_TEXT_ALPHA * fadeAlpha);

    ImVec2 bgMin(textPos.x - RenderingLayout::SUMMARY_BG_PADDING_X, textPos.y - RenderingLayout::SUMMARY_BG_PADDING_Y);
    ImVec2 bgMax(textPos.x + totalWidth + RenderingLayout::SUMMARY_BG_PADDING_X, textPos.y + font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, " ").y + RenderingLayout::SUMMARY_BG_PADDING_Y);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::SUMMARY_BG_ROUNDING);

    drawList->AddText(font, fontSize, ImVec2(textPos.x + RenderingLayout::TEXT_SHADOW_OFFSET, textPos.y + RenderingLayout::TEXT_SHADOW_OFFSET), IM_COL32(0, 0, 0, shadowAlpha), summaryText.c_str());
    drawList->AddText(font, fontSize, textPos, IM_COL32(200, 210, 255, textAlpha), summaryText.c_str());
}

} // namespace kx
