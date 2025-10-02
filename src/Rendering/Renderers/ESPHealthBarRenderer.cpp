#include "ESPHealthBarRenderer.h"
#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

void ESPHealthBarRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                                     float healthPercent, unsigned int entityColor,
                                                     float barWidth, float barHeight,
                                                     ESPEntityType entityType, Game::Attitude attitude) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Position the health bar below the entity center
    const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET;
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);

    // Background with subtle transparency and distance fade
    float bgAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha;
    unsigned int bgAlpha = static_cast<unsigned int>(bgAlphaf + 0.5f);
    bgAlpha = (bgAlpha > 255) ? 255 : bgAlpha;
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // Health fill dimensions
    float healthWidth = barWidth * healthPercent;
    ImVec2 healthBarMin(barMin.x, barMin.y);
    ImVec2 healthBarMax(barMin.x + healthWidth, barMax.y);

    // Calculate health bar color based on entity type and attitude (informative, not health-based)
    // Extract RGB from entityColor (which comes from ESPColors constants) and apply health bar alpha
    float healthAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha;
    unsigned int healthAlpha = static_cast<unsigned int>(healthAlphaf + 0.5f);
    healthAlpha = (healthAlpha > 255) ? 255 : healthAlpha;
    
    int r = (entityColor >> 16) & 0xFF;
    int g = (entityColor >> 8) & 0xFF;
    int b = entityColor & 0xFF;
    unsigned int healthColor = IM_COL32(b, g, r, healthAlpha);
    
    // Draw health fill
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // Minimal black border for hostile enemies only
    if (entityType == ESPEntityType::NPC && attitude == Game::Attitude::Hostile) {
        float borderAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha;
        unsigned int borderAlpha = static_cast<unsigned int>(borderAlphaf + 0.5f);
        borderAlpha = (borderAlpha > 255) ? 255 : borderAlpha;
        
        // Subtle black border for definition without being intrusive
        drawList->AddRect(barMin, barMax, IM_COL32(0, 0, 0, borderAlpha), 
                         RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ROUNDING, 0, 
                         RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS);
    }
}

} // namespace kx
