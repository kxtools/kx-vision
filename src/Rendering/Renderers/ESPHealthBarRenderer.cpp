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
    float healthAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha;
    unsigned int healthAlpha = static_cast<unsigned int>(healthAlphaf + 0.5f);
    healthAlpha = (healthAlpha > 255) ? 255 : healthAlpha;
    unsigned int baseHealthColor = (entityColor & 0x00FFFFFF) | (healthAlpha << 24);

    // --- Draw the BASE health fill (always draw the full bar in its normal color) ---
    drawList->AddRectFilled(healthBarMin, healthBarMax, baseHealthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    
    const RenderableEntity* entity = context.entity;
    if (!entity || entity->maxHealth <= 0) return;

    const EntityCombatState* state = stateManager.GetState(entity->address);
    if (!state) return;

    uint64_t now = GetTickCount64();

    // --- NEW: HEAL ABSORPTION OVERLAY LOGIC ---
    if (state->lastHealTimestamp > 0 && (now - state->lastHealTimestamp < CombatEffects::HEAL_OVERLAY_DURATION_MS)) {
        // Calculate the start and end percentages for the overlay
        float healStartPercent = state->healStartHealth / entity->maxHealth;
        float currentHealthPercent = entity->currentHealth / entity->maxHealth;

        // Don't draw if the heal was negligible or there's an issue
        if (currentHealthPercent > healStartPercent) {
            // Define the vibrant green overlay color
            ImU32 healOverlayColor = IM_COL32(100, 255, 100, 200); // Vibrant green with some transparency

            // Calculate the screen coordinates for the overlay segment
            ImVec2 healOverlayMin(barMin.x + barWidth * healStartPercent, barMin.y);
            ImVec2 healOverlayMax(barMin.x + barWidth * currentHealthPercent, barMax.y);

            // Draw the green overlay ON TOP of the base health bar
            drawList->AddRectFilled(healOverlayMin, healOverlayMax, healOverlayColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
        }
    }

    // --- NEW: HEAL FLASH EVENT LOGIC ---
    // This draws ON TOP of the overlay for maximum punch
    if (state->lastHealFlashTimestamp > 0 && (now - state->lastHealFlashTimestamp < CombatEffects::HEAL_FLASH_DURATION_MS)) {
        uint64_t timeSinceHealFlash = now - state->lastHealFlashTimestamp;
        float fadeProgress = (float)timeSinceHealFlash / CombatEffects::HEAL_FLASH_DURATION_MS;
        float flashAlpha = 1.0f - fadeProgress; // A simple linear fade is fine for a fast flash

        // Use a bright, almost pure white-cyan for the flash color. It will pop against the green overlay.
        ImU32 healFlashColor = IM_COL32(200, 255, 255, (int)(flashAlpha * 255));

        // The flash covers the same area as the overlay.
        float healStartPercent = state->healStartHealth / entity->maxHealth;
        float currentHealthPercent = entity->currentHealth / entity->maxHealth;

        if (currentHealthPercent > healStartPercent) {
            ImVec2 healOverlayMin(barMin.x + barWidth * healStartPercent, barMin.y);
            ImVec2 healOverlayMax(barMin.x + barWidth * currentHealthPercent, barMax.y);
            
            drawList->AddRectFilled(healOverlayMin, healOverlayMax, healFlashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
        }
    }

    // --- DAMAGE FLASH LOGIC (draws on top of everything else for max visibility) ---
    if (state->lastHitTimestamp > 0 && (now - state->lastHitTimestamp < CombatEffects::DAMAGE_FLASH_DURATION_MS)) {
        uint64_t timeSinceHit = now - state->lastHitTimestamp;
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