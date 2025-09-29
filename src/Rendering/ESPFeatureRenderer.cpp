#include "ESPFeatureRenderer.h"
#include "../../libs/ImGui/imgui.h"
#include <algorithm>

#include "ESPStyling.h"

namespace kx {

void ESPFeatureRenderer::RenderAttachedHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, 
                                                 float healthPercent, float fadeAlpha) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    const float barWidth = 4.0f;
    const float barHeight = boxMax.y - boxMin.y;
    
    ImVec2 barMin(boxMin.x - barWidth - 2.0f, boxMin.y);
    ImVec2 barMax(boxMin.x - 2.0f, boxMax.y);
    
    // Background with fade alpha
    unsigned int bgAlpha = static_cast<unsigned int>(150 * fadeAlpha);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha));
    
    // Health bar - fill from bottom to top with fade alpha
    ImVec2 healthBarMin(barMin.x, barMax.y - (barHeight * healthPercent));
    ImVec2 healthBarMax(barMax.x, barMax.y);
    unsigned int healthAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    unsigned int healthColor = IM_COL32(
        static_cast<int>(255 * (1.0f - healthPercent)),
        static_cast<int>(255 * healthPercent),
        0, healthAlpha
    );
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor);
    
    // Border with fade alpha
    unsigned int borderAlpha = static_cast<unsigned int>(100 * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32(255, 255, 255, borderAlpha));
}

void ESPFeatureRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos, 
                                                   float healthPercent, unsigned int entityColor) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Health bar dimensions - more natural looking
    const float barWidth = 40.0f;
    const float barHeight = 6.0f;
    const float yOffset = 15.0f; // Distance below the center point
    
    // Position the health bar below the entity center
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);
    
    // Background with subtle transparency and distance fade
    unsigned int bgAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha), 1.0f);
    
    // Health fill - horizontal bar
    float healthWidth = barWidth * healthPercent;
    ImVec2 healthBarMin(barMin.x, barMin.y);
    ImVec2 healthBarMax(barMin.x + healthWidth, barMax.y);
    
    // Health color: green -> yellow -> red based on percentage with distance fade
    unsigned int healthAlpha = static_cast<unsigned int>(160 * fadeAlpha);
    unsigned int healthColor;
    if (healthPercent > 0.66f) {
        // Green to yellow transition
        float t = (1.0f - healthPercent) / 0.34f;
        healthColor = IM_COL32(static_cast<int>(255 * t), 255, 0, healthAlpha);
    } else if (healthPercent > 0.33f) {
        // Yellow to orange transition
        healthColor = IM_COL32(255, static_cast<int>(255 * (healthPercent - 0.33f) / 0.33f), 0, healthAlpha);
    } else {
        // Red
        healthColor = IM_COL32(255, 0, 0, healthAlpha);
    }
    
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor, 1.0f);
    
    // Subtle border using entity color for identification with distance fade
    unsigned int borderAlpha = static_cast<unsigned int>(80 * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32((entityColor >> 16) & 0xFF, (entityColor >> 8) & 0xFF, entityColor & 0xFF, borderAlpha), 1.0f, 0, 1.0f);
}

void ESPFeatureRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos, 
                                         const std::string& playerName, unsigned int entityColor) {
    if (playerName.empty()) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Calculate text size and position
    ImVec2 textSize = ImGui::CalcTextSize(playerName.c_str());
    
    // Position name just below the feet position (below health bar area)
    const float nameOffset = 25.0f; // Below feet with more padding to avoid health bar overlap
    ImVec2 textPos(feetPos.x - textSize.x / 2, feetPos.y + nameOffset);
    
    // Extract RGB from entity color for a natural look
    int r = (entityColor >> 16) & 0xFF;
    int g = (entityColor >> 8) & 0xFF;
    int b = entityColor & 0xFF;
    
    // Subtle background with rounded corners (like game UI) and distance fade
    ImVec2 bgMin(textPos.x - 4, textPos.y - 2);
    ImVec2 bgMax(textPos.x + textSize.x + 4, textPos.y + textSize.y + 2);
    unsigned int bgAlpha = static_cast<unsigned int>(100 * fadeAlpha);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), 3.0f);
    
    // Subtle border using entity color with distance fade
    unsigned int borderAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddRect(bgMin, bgMax, IM_COL32(r, g, b, borderAlpha), 3.0f, 0, 1.0f);
    
    // Player name text in a clean, readable color with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(220 * fadeAlpha);
    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), playerName.c_str()); // Shadow
    drawList->AddText(textPos, IM_COL32(255, 255, 255, textAlpha), playerName.c_str()); // Main text
}

void ESPFeatureRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, 
                                          unsigned int color) {
    // Main box
    drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, 2.0f);
    
    // Corner indicators for better visibility
    const float cornerSize = 8.0f;
    const float thickness = 2.0f;
    
    // Top-left corner
    drawList->AddLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x + cornerSize, boxMin.y), color, thickness);
    drawList->AddLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x, boxMin.y + cornerSize), color, thickness);
    
    // Top-right corner
    drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x - cornerSize, boxMin.y), color, thickness);
    drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x, boxMin.y + cornerSize), color, thickness);
    
    // Bottom-left corner
    drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x + cornerSize, boxMax.y), color, thickness);
    drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x, boxMax.y - cornerSize), color, thickness);
    
    // Bottom-right corner
    drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x - cornerSize, boxMax.y), color, thickness);
    drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x, boxMax.y - cornerSize), color, thickness);
}

void ESPFeatureRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, 
                                           float distance, float fadeAlpha) {
    char distText[32];
    snprintf(distText, sizeof(distText), "%.1fm", distance);
    
    ImVec2 textSize = ImGui::CalcTextSize(distText);
    ImVec2 textPos(center.x - textSize.x / 2, boxMin.y - textSize.y - 5);
    
    // Background with distance fade - REDUCED OPACITY
    unsigned int bgAlpha = static_cast<unsigned int>(100 * fadeAlpha); // Before: 150
    drawList->AddRectFilled(ImVec2(textPos.x - 2, textPos.y - 1),
        ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1),
        IM_COL32(0, 0, 0, bgAlpha), 2.0f);

    // Text with shadow and distance fade - REDUCED OPACITY
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha); // Before: 255
    unsigned int textAlpha = static_cast<unsigned int>(220 * fadeAlpha);   // Before: 255
    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), distText);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, textAlpha), distText);
}

void ESPFeatureRenderer::RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos, unsigned int color) {
    // Extract fade alpha from the color parameter
    float fadeAlpha = ((color >> 24) & 0xFF) / 255.0f;
    
    // Small, minimalistic dot with subtle outline for visibility
    // Dark outline with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), 2.5f, IM_COL32(0, 0, 0, shadowAlpha));
    // Main dot using entity color (already has faded alpha)
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), 2.0f, color);
}

void ESPFeatureRenderer::RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos, float fadeAlpha) {
    ImVec2 pos(feetPos.x, feetPos.y);
    
    // Shadow with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(pos.x + 1, pos.y + 1), 2.0f, IM_COL32(0, 0, 0, shadowAlpha));
    
    // Dot with distance fade
    unsigned int dotAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    drawList->AddCircleFilled(pos, 1.5f, IM_COL32(255, 255, 255, dotAlpha));
}

void ESPFeatureRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax,
    const std::vector<ColoredDetail>& details, float fadeAlpha) {
    if (details.empty()) return;

    float textY = boxMax.y + 5.0f;

    for (const auto& detail : details) {
        ImVec2 textSize = ImGui::CalcTextSize(detail.text.c_str());
        ImVec2 textPos(center.x - textSize.x / 2, textY);

        // Background with distance fade - REDUCED OPACITY
        unsigned int bgAlpha = static_cast<unsigned int>(100 * fadeAlpha); // Before: 160
        drawList->AddRectFilled(ImVec2(textPos.x - 3, textPos.y - 1),
            ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 1),
            IM_COL32(0, 0, 0, bgAlpha), 1.0f);

        // Text with shadow and distance fade - REDUCED SHADOW OPACITY
        unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha); // Before: 200

        // Extract the original alpha from the detail color and combine it with the fade alpha
        int originalAlpha = (detail.color >> 24) & 0xFF;
        unsigned int textAlpha = static_cast<unsigned int>(originalAlpha * fadeAlpha);

        // Re-create the final text color with the new combined alpha
        ImU32 finalTextColor = (detail.color & 0x00FFFFFF) | (textAlpha << 24);

        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), detail.text.c_str());
        drawList->AddText(textPos, finalTextColor, detail.text.c_str());

        textY += textSize.y + 3;
    }
}

unsigned int ESPFeatureRenderer::ApplyAlphaToColor(unsigned int color, float alpha) {
    // Extract RGBA components
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    int originalAlpha = (color >> 24) & 0xFF;
    
    // Apply alpha multiplier while preserving original alpha intentions
    int newAlpha = static_cast<int>(originalAlpha * alpha);
    newAlpha = (newAlpha < 0) ? 0 : (newAlpha > 255) ? 255 : newAlpha; // Clamp to valid range
    
    return IM_COL32(r, g, b, newAlpha);
}

void ESPFeatureRenderer::RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
    const std::vector<CompactStatInfo>& summary, float fadeAlpha) {
    if (summary.empty()) return;

    // --- Part 1: Calculate total width for centering ---
    float totalWidth = 0.0f;
    const std::string prefix = "Stats: ";
    totalWidth += ImGui::CalcTextSize(prefix.c_str()).x;

    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        std::string segment = std::to_string(info.count) + "x " + info.statName;
        totalWidth += ImGui::CalcTextSize(segment.c_str()).x;
        if (i < summary.size() - 1) {
            totalWidth += ImGui::CalcTextSize(", ").x;
        }
    }

    // --- Part 2: Render the multi-colored line ---
    const float summaryOffset = 42.0f;
    ImVec2 currentPos(feetPos.x - totalWidth / 2.0f, feetPos.y + summaryOffset);

    // Fade alphas
    unsigned int bgAlpha = static_cast<unsigned int>(80 * fadeAlpha);
    unsigned int shadowAlpha = static_cast<unsigned int>(160 * fadeAlpha);
    unsigned int defaultTextAlpha = static_cast<unsigned int>(200 * fadeAlpha);

    // Background
    ImVec2 bgMin(currentPos.x - 4, currentPos.y - 2);
    ImVec2 bgMax(currentPos.x + totalWidth + 4, currentPos.y + ImGui::GetTextLineHeight() + 2);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), 3.0f);

    // Render "Stats: " prefix in default color
    drawList->AddText(ImVec2(currentPos.x + 1, currentPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), prefix.c_str());
    drawList->AddText(currentPos, IM_COL32(200, 210, 255, defaultTextAlpha), prefix.c_str());
    currentPos.x += ImGui::CalcTextSize(prefix.c_str()).x;

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

        drawList->AddText(ImVec2(currentPos.x + 1, currentPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), segment.c_str());
        drawList->AddText(currentPos, rarityColor, segment.c_str());
        currentPos.x += ImGui::CalcTextSize(segment.c_str()).x;

        // Render separator
        if (i < summary.size() - 1) {
            const std::string separator = ", ";
            drawList->AddText(ImVec2(currentPos.x + 1, currentPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), separator.c_str());
            drawList->AddText(currentPos, IM_COL32(200, 210, 255, defaultTextAlpha), separator.c_str());
            currentPos.x += ImGui::CalcTextSize(separator.c_str()).x;
        }
    }
}

} // namespace kx