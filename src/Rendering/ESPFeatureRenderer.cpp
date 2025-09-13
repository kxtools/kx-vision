#include "ESPFeatureRenderer.h"
#include "../../libs/ImGui/imgui.h"
#include <algorithm>

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
    
    // Background with distance fade
    unsigned int bgAlpha = static_cast<unsigned int>(150 * fadeAlpha);
    drawList->AddRectFilled(ImVec2(textPos.x - 2, textPos.y - 1), 
                          ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1), 
                          IM_COL32(0, 0, 0, bgAlpha), 2.0f);
    
    // Text with shadow and distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(255 * fadeAlpha);
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
                                          const std::vector<std::string>& details, float fadeAlpha) {
    if (details.empty()) return;

    float textY = boxMax.y + 5.0f;
    
    for (const auto& detail : details) {
        ImVec2 textSize = ImGui::CalcTextSize(detail.c_str());
        ImVec2 textPos(center.x - textSize.x / 2, textY);
        
        // Background with distance fade
        unsigned int bgAlpha = static_cast<unsigned int>(160 * fadeAlpha);
        drawList->AddRectFilled(ImVec2(textPos.x - 3, textPos.y - 1), 
                              ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 1), 
                              IM_COL32(0, 0, 0, bgAlpha), 1.0f);
        
        // Text with shadow and distance fade
        unsigned int shadowAlpha = static_cast<unsigned int>(200 * fadeAlpha);
        unsigned int textAlpha = static_cast<unsigned int>(255 * fadeAlpha);
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), detail.c_str());
        drawList->AddText(textPos, IM_COL32(255, 255, 255, textAlpha), detail.c_str());
        
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

} // namespace kx