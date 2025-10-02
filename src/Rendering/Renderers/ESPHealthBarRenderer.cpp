#include "ESPHealthBarRenderer.h"
#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

void ESPHealthBarRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                                     float healthPercent, unsigned int entityColor,
                                                     float barWidth, float barHeight) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Position the health bar below the entity center
    const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET;
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);

    // Background with subtle transparency and distance fade
    float bgAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha;
    unsigned int bgAlpha = static_cast<unsigned int>(bgAlphaf + 0.5f); // Round for smooth fading
    bgAlpha = (bgAlpha > 255) ? 255 : bgAlpha;
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // Health fill - horizontal bar
    float healthWidth = barWidth * healthPercent;
    ImVec2 healthBarMin(barMin.x, barMin.y);
    ImVec2 healthBarMax(barMin.x + healthWidth, barMax.y);

    // Health color: green -> yellow -> orange based on percentage with smooth distance fade
    float healthAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha;
    unsigned int healthAlpha = static_cast<unsigned int>(healthAlphaf + 0.5f); // Round for smooth transitions
    healthAlpha = (healthAlpha > 255) ? 255 : healthAlpha;
    
    unsigned int healthColor;
    if (healthPercent > 0.66f) {
        // Green to yellow transition - high health (100% → 66%)
        float t = (1.0f - healthPercent) / 0.34f;
        int red = static_cast<int>(255.0f * t + 0.5f);
        healthColor = IM_COL32(red, 255, 0, healthAlpha);
    } else if (healthPercent > 0.33f) {
        // Yellow to orange transition - medium health (66% → 33%)
        // At 66%: (255, 255, 0) yellow
        // At 33%: (255, 165, 0) orange
        float t = (healthPercent - 0.33f) / 0.33f; // 0.0 at 33%, 1.0 at 66%
        int green = static_cast<int>((165.0f + (255.0f - 165.0f) * t) + 0.5f); // Interpolate 165 → 255
        healthColor = IM_COL32(255, green, 0, healthAlpha);
    } else {
        // Orange for critical health (below 33%)
        healthColor = IM_COL32(255, 165, 0, healthAlpha);
    }
    
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // Subtle border using entity color for identification - this makes it feel natural and tied to the entity
    float borderAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha;
    unsigned int borderAlpha = static_cast<unsigned int>(borderAlphaf + 0.5f);
    borderAlpha = (borderAlpha > 255) ? 255 : borderAlpha;
    
    // Extract RGB from entity color
    int r = (entityColor >> 16) & 0xFF;
    int g = (entityColor >> 8) & 0xFF;
    int b = entityColor & 0xFF;
    
    drawList->AddRect(barMin, barMax, IM_COL32(r, g, b, borderAlpha), 
                     RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ROUNDING, 0, 
                     RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS);
}

} // namespace kx
