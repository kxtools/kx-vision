#include "ESPHealthBarRenderer.h"
#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include <Windows.h>
#include "../Combat/CombatStateManager.h"
#include "../Data/EntityRenderContext.h"
#include <algorithm>
#include <cmath>
#include "../Utils/AnimationHelpers.h"

namespace kx {

void ESPHealthBarRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                                     const EntityRenderContext& context, unsigned int entityColor,
                                                     float barWidth, float barHeight,
                                                     const CombatStateManager& stateManager) {
    if (context.healthPercent < -1.0f) return; // Allow 0 health for dead entities

    // --- 1. Initial Setup (The Controller's Job) ---
    const RenderableEntity* entity = context.entity;
    const EntityCombatState* state = stateManager.GetState(entity ? entity->address : nullptr);
    uint64_t now = GetTickCount64();
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;
    float timeFade = 1.0f;

    // --- 2. Handle Final Fade-out After Death ---
    if (state && state->deathTimestamp > 0) {
        uint64_t timeSinceDeath = now - state->deathTimestamp;
        if (timeSinceDeath > CombatEffects::DEATH_BURST_DURATION_MS) {
            uint64_t timeIntoFade = timeSinceDeath - CombatEffects::DEATH_BURST_DURATION_MS;
            if (timeIntoFade < CombatEffects::DEATH_FINAL_FADE_DURATION_MS) {
                timeFade = 1.0f - ((float)timeIntoFade / CombatEffects::DEATH_FINAL_FADE_DURATION_MS);
            } else {
                timeFade = 0.0f;
            }
        }
    }
    if (timeFade <= 0.0f) return;
    fadeAlpha *= timeFade;

    // --- 3. Draw Common Elements ---
    const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET;
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);
    unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha + 0.5f);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, ClampAlpha(bgAlpha)), RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // --- 4. Delegate to the Correct Specialist ---
    if (state && state->deathTimestamp > 0) {
        RenderDeadState(drawList, state, barMin, barMax, barWidth, fadeAlpha);
    } else {
        RenderAliveState(drawList, context, state, barMin, barMax, barWidth, entityColor, fadeAlpha);
    }

    // --- 5. Draw Final Border ---
    if (context.attitude == Game::Attitude::Hostile) {
        unsigned int borderAlpha = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha + 0.5f);
        drawList->AddRect(barMin, barMax, IM_COL32(0, 0, 0, ClampAlpha(borderAlpha)), 
                         RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING, 0, 
                         RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS);
    }
}

// --- NEW, SPECIALIST HELPER FUNCTIONS ---

void ESPHealthBarRenderer::RenderAliveState(ImDrawList* drawList, const EntityRenderContext& context, const EntityCombatState* state,
                                            const ImVec2& barMin, const ImVec2& barMax, float barWidth, unsigned int entityColor, float fadeAlpha) {
    const RenderableEntity* entity = context.entity;
    if (!entity || entity->maxHealth <= 0) return;

    // Calculate base color
    float healthWidth = barWidth * context.healthPercent;
    ImVec2 healthBarMin = barMin;
    ImVec2 healthBarMax(barMin.x + healthWidth, barMax.y);
    unsigned int healthAlpha = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha + 0.5f);
    unsigned int baseHealthColor = (entityColor & 0x00FFFFFF) | (ClampAlpha(healthAlpha) << 24);

    // Draw base health fill
    drawList->AddRectFilled(healthBarMin, healthBarMax, baseHealthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    
    if (!state) return; // No state, no effects
    uint64_t now = GetTickCount64();

    // Render Heal Absorption Overlay
    if (state->lastHealTimestamp > 0 && (now - state->lastHealTimestamp < CombatEffects::HEAL_OVERLAY_DURATION_MS)) {
        uint64_t timeSinceHeal = now - state->lastHealTimestamp;

        // --- NEW FADE-OUT LOGIC ---
        float overlayAlpha = 1.0f; // Start with full alpha
        uint64_t fadeStartTime = CombatEffects::HEAL_OVERLAY_DURATION_MS - CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;

        if (timeSinceHeal > fadeStartTime) {
            // We are in the fade period.
            uint64_t timeIntoFade = timeSinceHeal - fadeStartTime;
            float fadeProgress = (float)timeIntoFade / CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;
            overlayAlpha = 1.0f - fadeProgress;
        }

        float healStartPercent = state->healStartHealth / entity->maxHealth;
        float currentHealthPercent = entity->currentHealth / entity->maxHealth;

        if (currentHealthPercent > healStartPercent) {
            // Apply the calculated alpha to the overlay color
            ImU32 healOverlayColor = IM_COL32(100, 255, 100, (int)(200 * overlayAlpha)); 

            ImVec2 healOverlayMin(barMin.x + barWidth * healStartPercent, barMin.y);
            ImVec2 healOverlayMax(barMin.x + barWidth * currentHealthPercent, barMax.y);
            drawList->AddRectFilled(healOverlayMin, healOverlayMax, healOverlayColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
        }
    }

    // Render Heal Flash
    if (state->lastHealFlashTimestamp > 0 && (now - state->lastHealFlashTimestamp < CombatEffects::HEAL_FLASH_DURATION_MS)) {
        uint64_t timeSinceHealFlash = now - state->lastHealFlashTimestamp;
        float linearProgress = (float)timeSinceHealFlash / CombatEffects::HEAL_FLASH_DURATION_MS;
        float flashAlpha = 1.0f - Animation::EaseOutCubic(linearProgress);
        ImU32 healFlashColor = IM_COL32(200, 255, 255, (int)(flashAlpha * 255));
        float healStartPercent = state->healStartHealth / entity->maxHealth;
        float currentHealthPercent = entity->currentHealth / entity->maxHealth;
        if (currentHealthPercent > healStartPercent) {
            ImVec2 healOverlayMin(barMin.x + barWidth * healStartPercent, barMin.y);
            ImVec2 healOverlayMax(barMin.x + barWidth * currentHealthPercent, barMax.y);
            drawList->AddRectFilled(healOverlayMin, healOverlayMax, healFlashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
        }
    }

    // Render Damage Flash
    if (state->lastHitTimestamp > 0 && (now - state->lastHitTimestamp < CombatEffects::DAMAGE_FLASH_DURATION_MS)) {
        uint64_t timeSinceHit = now - state->lastHitTimestamp;
        float linearProgress = (float)timeSinceHit / CombatEffects::DAMAGE_FLASH_DURATION_MS;
        float flashAlpha = 1.0f - Animation::EaseOutCubic(linearProgress);
        ImU32 flashColor = IM_COL32(255, 255, 0, (int)(flashAlpha * 255));
        float currentHealthPercent = entity->currentHealth / entity->maxHealth;
        float previousHealthPercent = (entity->currentHealth + state->lastDamageTaken) / entity->maxHealth;
        ImVec2 flashMin = ImVec2(barMin.x + barWidth * currentHealthPercent, barMin.y);
        ImVec2 flashMax = ImVec2(barMin.x + barWidth * (previousHealthPercent < 1.0f ? previousHealthPercent : 1.0f), barMax.y);
        if (flashMin.x < flashMax.x) {
            drawList->AddRectFilled(flashMin, flashMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
        }
    }
}

void ESPHealthBarRenderer::RenderDeadState(ImDrawList* drawList, const EntityCombatState* state,
                                           const ImVec2& barMin, const ImVec2& barMax, float barWidth, float fadeAlpha) {
    uint64_t now = GetTickCount64();
    uint64_t timeSinceDeath = now - state->deathTimestamp;

    // Render Energy Burst
    if (timeSinceDeath < CombatEffects::DEATH_BURST_DURATION_MS) {
        float linearProgress = (float)timeSinceDeath / CombatEffects::DEATH_BURST_DURATION_MS;

        // --- APPLY EASING ---
        float easedProgress = Animation::EaseOutCubic(linearProgress);
        
        float burstAlpha = 1.0f - linearProgress; // Keep the fade linear so it doesn't disappear too fast
        float currentBurstWidth = barWidth * easedProgress; // The burst width now follows the eased curve!

        ImU32 burstColor = IM_COL32(200, 255, 255, (int)(burstAlpha * 255 * fadeAlpha));
        float barHeight = barMax.y - barMin.y;
        
        ImVec2 centerPoint(barMin.x + barWidth * 0.5f, barMin.y + barHeight * 0.5f);
        ImVec2 burstMin(centerPoint.x - currentBurstWidth * 0.5f, barMin.y);
        ImVec2 burstMax(centerPoint.x + currentBurstWidth * 0.5f, barMax.y);
        
        drawList->AddRectFilled(burstMin, burstMax, burstColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
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
    bgAlpha = ClampAlpha(bgAlpha);
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
    finalAlpha = ClampAlpha(finalAlpha);
    unsigned int finalColor = (energyColor & 0x00FFFFFF) | (finalAlpha << 24);
    
    drawList->AddRectFilled(energyBarMin, energyBarMax, finalColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
}

unsigned int ESPHealthBarRenderer::ClampAlpha(unsigned int alpha) {
    return (alpha < 255u ? alpha : 255u);
}

} // namespace kx
