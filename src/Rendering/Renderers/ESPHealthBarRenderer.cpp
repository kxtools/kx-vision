#include "ESPHealthBarRenderer.h"
#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include <Windows.h>
#include "../Combat/CombatStateManager.h"
#include "../Data/EntityRenderContext.h"
#include <algorithm>
#include <cmath>

namespace kx {

void ESPHealthBarRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                                     const EntityRenderContext& context, unsigned int entityColor,
                                                     float barWidth, float barHeight,
                                                     const CombatStateManager& stateManager) {
    if (context.healthPercent < 0.0f || context.healthPercent > 1.0f) return;

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
    float healthWidth = barWidth * context.healthPercent;
    ImVec2 healthBarMin(barMin.x, barMin.y);
    ImVec2 healthBarMax(barMin.x + healthWidth, barMax.y);

    // Calculate health bar color - use entityColor directly with health bar alpha
    // EntityColor already comes from ESPColors constants with proper RGB values
    float healthAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha;
    unsigned int healthAlpha = static_cast<unsigned int>(healthAlphaf + 0.5f);
    healthAlpha = (healthAlpha > 255) ? 255 : healthAlpha;
    
    // Replace alpha component with health bar alpha, keep RGB unchanged
    unsigned int healthColor = (entityColor & 0x00FFFFFF) | (healthAlpha << 24);

    // --- NEW HEALING PULSE LOGIC ---
    const RenderableEntity* entity = context.entity;
    if (!entity) return; // Should not happen if health bar is drawn
    const EntityCombatState* state = stateManager.GetState(entity->address);
    if (state && state->lastHealTimestamp > 0) {
        uint64_t timeSinceHeal = GetTickCount64() - state->lastHealTimestamp;

        if (timeSinceHeal < CombatEffects::HEAL_PULSE_DURATION_MS) {
            // Use a sine wave that completes one full pulse over the duration
            float pulseProgress = (float)timeSinceHeal / CombatEffects::HEAL_PULSE_DURATION_MS;
            float pulse = sin(pulseProgress * 3.14159f); // A single hump from 0 -> 1 -> 0

            // Interpolate the health bar color towards a bright, vibrant green
            int r = (healthColor >> 0) & 0xFF;
            int g = (healthColor >> 8) & 0xFF;
            int b = (healthColor >> 16) & 0xFF;
            int a = (healthColor >> 24) & 0xFF;

            r -= (int)(r * pulse * 0.8f); // Pull red down
            g = g + (int)((255 - g) * pulse); // Push green to max
            b -= (int)(b * pulse * 0.8f); // Pull blue down

            healthColor = IM_COL32(r, g, b, a);
        }
    }
    
    // Draw health fill
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // --- ENHANCED DAMAGE FLASH LOGIC ---
    if (state && state->lastHitTimestamp > 0) {
        uint64_t timeSinceHit = GetTickCount64() - state->lastHitTimestamp;

        if (timeSinceHit < CombatEffects::DAMAGE_FLASH_DURATION_MS) {
            float fadeProgress = (float)timeSinceHit / CombatEffects::DAMAGE_FLASH_DURATION_MS;
            float flashAlpha = 1.0f - (fadeProgress * fadeProgress);

            ImU32 flashColor = IM_COL32(255, 255, 0, (int)(flashAlpha * 255));

            float currentHealthPercent = entity->currentHealth / entity->maxHealth;
            float previousHealthPercent = (entity->currentHealth + state->lastDamageTaken) / entity->maxHealth;

            ImVec2 flashMin = ImVec2(barMin.x + barWidth * currentHealthPercent, barMin.y);
            ImVec2 flashMax = ImVec2(barMin.x + barWidth * (std::min)(1.0f, previousHealthPercent), barMax.y);

            if (flashMin.x < flashMax.x) {
                drawList->AddRectFilled(flashMin, flashMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
            }
        }
    }

    // Border for hostile entities - simple black border
    if (context.attitude == Game::Attitude::Hostile) {
        float borderAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha;
        unsigned int borderAlpha = static_cast<unsigned int>(borderAlphaf + 0.5f);
        borderAlpha = (borderAlpha > 255) ? 255 : borderAlpha;
        
        // Subtle black border for definition without being intrusive
        drawList->AddRect(barMin, barMax, IM_COL32(0, 0, 0, borderAlpha), 
                         RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ROUNDING, 0, 
                         RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS);
    }
}

void ESPHealthBarRenderer::RenderStandaloneEnergyBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                                     float energyPercent, float fadeAlpha,
                                                     float barWidth, float barHeight, float healthBarHeight) {
    if (energyPercent < 0.0f || energyPercent > 1.0f) return;

    // Position below health bar, with a 2px gap
    const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET + healthBarHeight + 2.0f;
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);

    // Background
    float bgAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha;
    unsigned int bgAlpha = static_cast<unsigned int>(bgAlphaf + 0.5f);
    bgAlpha = (bgAlpha > 255) ? 255 : bgAlpha;
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // Energy fill
    float energyWidth = barWidth * energyPercent;
    ImVec2 energyBarMin(barMin.x, barMin.y);
    ImVec2 energyBarMax(barMin.x + energyWidth, barMax.y);

    // Color
    unsigned int energyColor = ESPColors::ENERGY_BAR;
    float colorAlphaF = ((energyColor >> 24) & 0xFF) / 255.0f; // Get alpha from the constant
    colorAlphaF *= fadeAlpha; // Apply distance fade
    unsigned int finalAlpha = static_cast<unsigned int>(colorAlphaF * 255.0f + 0.5f);
    finalAlpha = (finalAlpha > 255) ? 255 : finalAlpha;
    unsigned int finalColor = (energyColor & 0x00FFFFFF) | (finalAlpha << 24);
    
    drawList->AddRectFilled(energyBarMin, energyBarMax, finalColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
}

} // namespace kx