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
    if (context.healthPercent < -1.0f) return; // Allow 0 health for dead entities

    // --- Initial Setup ---
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;
    float timeFade = 1.0f;

    const RenderableEntity* entity = context.entity;
    const EntityCombatState* state = stateManager.GetState(entity ? entity->address : nullptr);
    uint64_t now = GetTickCount64();

    // --- CHECK IF ENTITY IS DEAD and handle final fade-out ---
    if (state && state->deathTimestamp > 0) {
        uint64_t timeSinceDeath = now - state->deathTimestamp;
        
        // Check if we are in the final fade-out period
        if (timeSinceDeath > CombatEffects::DEATH_EMBER_FADE_DURATION_MS) {
            uint64_t timeIntoFade = timeSinceDeath - CombatEffects::DEATH_EMBER_FADE_DURATION_MS;
            if (timeIntoFade < CombatEffects::DEATH_FINAL_FADE_DURATION_MS) {
                float fadeProgress = (float)timeIntoFade / CombatEffects::DEATH_FINAL_FADE_DURATION_MS;
                timeFade = 1.0f - fadeProgress;
            } else {
                timeFade = 0.0f; // Fully faded
            }
        }
    }

    if (timeFade <= 0.0f) return; // Early exit if fully faded

    // Apply time fade to overall alpha
    fadeAlpha *= timeFade;

    // --- Drawing ---
    const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET;
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);
    float bgAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha;
    unsigned int bgAlpha = static_cast<unsigned int>(bgAlphaf + 0.5f);
    bgAlpha = (bgAlpha > 255) ? 255 : bgAlpha;

    // --- Draw the Background (always visible, alive or dead) ---
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha), RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // --- Handle Dead/Alive states ---
    if (state && state->deathTimestamp > 0) {
        uint64_t timeSinceDeath = now - state->deathTimestamp;

        // --- PHASE 1: THE SHATTER ---
        if (timeSinceDeath < CombatEffects::DEATH_SHATTER_DURATION_MS) {
            float fadeProgress = (float)timeSinceDeath / CombatEffects::DEATH_SHATTER_DURATION_MS;
            float flashAlpha = 1.0f - fadeProgress;
            ImU32 shatterColor = IM_COL32(255, 255, 255, (int)(flashAlpha * 255));
            float thickness = 2.0f;

            // Draw a bright, fading 'X' over the empty bar
            drawList->AddLine(barMin, barMax, shatterColor, thickness);
            drawList->AddLine(ImVec2(barMin.x, barMax.y), ImVec2(barMax.x, barMin.y), shatterColor, thickness);
        }
        // --- PHASE 2: THE EMBER ---
        else if (timeSinceDeath < CombatEffects::DEATH_EMBER_FADE_DURATION_MS) {
            float fadeProgress = (float)timeSinceDeath / CombatEffects::DEATH_EMBER_FADE_DURATION_MS;
            float emberAlpha = 1.0f - fadeProgress;

            // Interpolate color from bright orange to dark red
            ImVec4 orange = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
            ImVec4 darkRed = ImVec4(0.4f, 0.0f, 0.0f, 1.0f);
            ImVec4 emberColorVec;
            emberColorVec.x = orange.x + (darkRed.x - orange.x) * fadeProgress;
            emberColorVec.y = orange.y + (darkRed.y - orange.y) * fadeProgress;
            emberColorVec.z = orange.z + (darkRed.z - orange.z) * fadeProgress;
            emberColorVec.w = emberAlpha;
            ImU32 emberColor = ImGui::ColorConvertFloat4ToU32(emberColorVec);

            // Draw a single-pixel line at the start of the bar
            drawList->AddLine(ImVec2(barMin.x, barMin.y), ImVec2(barMin.x, barMax.y), emberColor, 1.5f);
        }
    }
    // --- ELSE, ENTITY IS ALIVE: RENDER NORMAL HEALTH BAR ---
    else {
        if (!entity || entity->maxHealth <= 0) return;

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

        if (!state) return;

        // --- HEAL ABSORPTION OVERLAY LOGIC ---
        if (state->lastHealTimestamp > 0 && (now - state->lastHealTimestamp < CombatEffects::HEAL_OVERLAY_DURATION_MS)) {
            float healStartPercent = state->healStartHealth / entity->maxHealth;
            float currentHealthPercent = entity->currentHealth / entity->maxHealth;
            if (currentHealthPercent > healStartPercent) {
                ImU32 healOverlayColor = IM_COL32(100, 255, 100, 200);
                ImVec2 healOverlayMin(barMin.x + barWidth * healStartPercent, barMin.y);
                ImVec2 healOverlayMax(barMin.x + barWidth * currentHealthPercent, barMax.y);
                drawList->AddRectFilled(healOverlayMin, healOverlayMax, healOverlayColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
            }
        }

        // --- HEAL FLASH EVENT LOGIC ---
        if (state->lastHealFlashTimestamp > 0 && (now - state->lastHealFlashTimestamp < CombatEffects::HEAL_FLASH_DURATION_MS)) {
            uint64_t timeSinceHealFlash = now - state->lastHealFlashTimestamp;
            float fadeProgress = (float)timeSinceHealFlash / CombatEffects::HEAL_FLASH_DURATION_MS;
            float flashAlpha = 1.0f - fadeProgress;
            ImU32 healFlashColor = IM_COL32(200, 255, 255, (int)(flashAlpha * 255));
            float healStartPercent = state->healStartHealth / entity->maxHealth;
            float currentHealthPercent = entity->currentHealth / entity->maxHealth;
            if (currentHealthPercent > healStartPercent) {
                ImVec2 healOverlayMin(barMin.x + barWidth * healStartPercent, barMin.y);
                ImVec2 healOverlayMax(barMin.x + barWidth * currentHealthPercent, barMax.y);
                drawList->AddRectFilled(healOverlayMin, healOverlayMax, healFlashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
            }
        }

        // --- DAMAGE FLASH LOGIC ---
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
    }

    // --- Draw the Border (always visible, alive or dead) ---
    if (context.attitude == Game::Attitude::Hostile) {
        float borderAlphaf = RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha;
        unsigned int borderAlpha = static_cast<unsigned int>(borderAlphaf + 0.5f);
        borderAlpha = (borderAlpha > 255) ? 255 : borderAlpha;
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