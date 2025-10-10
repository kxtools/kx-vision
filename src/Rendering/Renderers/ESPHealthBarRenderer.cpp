#define NOMINMAX

#include "ESPHealthBarRenderer.h"

#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include <Windows.h>
#include "../Combat/CombatStateManager.h"
#include "../Data/EntityRenderContext.h"
#include <algorithm>
#include "../Utils/AnimationHelpers.h"

namespace kx {

    // -----------------------------------------------------------------------------
    // Small Utilities
    // -----------------------------------------------------------------------------
    ImU32 ESPHealthBarRenderer::ApplyAlphaToColor(ImU32 color, float alphaMul) {
        alphaMul = (alphaMul < 0.f ? 0.f : (alphaMul > 1.f ? 1.f : alphaMul));
        unsigned int a = (color >> 24) & 0xFF;
        unsigned int finalA = ClampAlpha(static_cast<unsigned int>(a * alphaMul + 0.5f));
        return (color & 0x00FFFFFF) | (finalA << 24);
    }

    uint64_t ESPHealthBarRenderer::NowMs() {
        return GetTickCount64();
    }

    void ESPHealthBarRenderer::DrawFilledRect(ImDrawList* dl,
        const ImVec2& min,
        const ImVec2& max,
        ImU32 color,
        float rounding) {
        if (min.x < max.x && min.y < max.y) {
            dl->AddRectFilled(min, max, color, rounding);
        }
    }

    void ESPHealthBarRenderer::DrawHealthBase(ImDrawList* dl,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        float healthPercent,
        unsigned int entityColor,
        float fadeAlpha) {
        float hpWidth = barWidth * Clamp01(healthPercent);
        ImVec2 hMin = barMin;
        ImVec2 hMax(barMin.x + hpWidth, barMax.y);

        unsigned int healthAlpha =
            static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha + 0.5f);
        unsigned int baseColorNoA = (entityColor & 0x00FFFFFF);
        ImU32 baseHealthColor = (baseColorNoA) | (ClampAlpha(healthAlpha) << 24);

        DrawFilledRect(dl, hMin, hMax, baseHealthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawHealOverlay(ImDrawList* dl,
        const EntityCombatState* state,
        const RenderableEntity* entity,
        uint64_t now,
        const ImVec2& barMin,
        float barWidth,
        float barHeight) {
        if (!state || !entity || entity->maxHealth <= 0) return;
        if (state->lastHealTimestamp == 0) return;
        uint64_t elapsed = now - state->lastHealTimestamp;
        if (elapsed >= CombatEffects::HEAL_OVERLAY_DURATION_MS) return;

        float overlayAlpha = 1.0f;
        if (CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS < CombatEffects::HEAL_OVERLAY_DURATION_MS) {
            uint64_t fadeStart = CombatEffects::HEAL_OVERLAY_DURATION_MS - CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;
            if (elapsed > fadeStart) {
                uint64_t intoFade = elapsed - fadeStart;
                float fadeProgress = static_cast<float>(intoFade) / CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;
                overlayAlpha = 1.0f - Animation::EaseOutCubic(fadeProgress);
            }
        }

        float startPercent = state->healStartHealth / entity->maxHealth;
        float currentPercent = entity->currentHealth / entity->maxHealth;
        if (currentPercent <= startPercent) return;

        ImVec2 oMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 oMax(barMin.x + barWidth * currentPercent, barMin.y + barHeight);

        ImU32 color = IM_COL32(100, 255, 100, static_cast<int>(200 * overlayAlpha));
        DrawFilledRect(dl, oMin, oMax, color, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawHealFlash(ImDrawList* dl,
        const EntityCombatState* state,
        const RenderableEntity* entity,
        uint64_t now,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha) {
        if (!state || !entity || entity->maxHealth <= 0) return;
        if (state->lastHealFlashTimestamp == 0) return;

        uint64_t elapsed = now - state->lastHealFlashTimestamp;
        if (elapsed >= CombatEffects::HEAL_FLASH_DURATION_MS) return;

        float linear = static_cast<float>(elapsed) / CombatEffects::HEAL_FLASH_DURATION_MS;
        if (linear > 1.f) linear = 1.f;
        float flashAlpha = 1.0f - Animation::EaseOutCubic(linear);

        float startPercent = state->healStartHealth / entity->maxHealth;
        float currentPercent = entity->currentHealth / entity->maxHealth;
        if (currentPercent <= startPercent) return;

        ImVec2 fMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 fMax(barMin.x + barWidth * currentPercent, barMin.y + barHeight);

        ImU32 flashColor = IM_COL32(200, 255, 255, static_cast<int>(flashAlpha * 255 * fadeAlpha));
        DrawFilledRect(dl, fMin, fMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawAccumulatedDamage(ImDrawList* dl,
        EntityCombatState* state,
        const RenderableEntity* entity,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha) {
        if (!state || !entity || entity->maxHealth <= 0) return;
        if (state->accumulatedDamage <= 0) return;

        uint64_t now = NowMs();
        float animationAlpha = 1.0f; // Start with full opacity

        // --- FADE-OUT ANIMATION LOGIC ---
        if (state->flushAnimationStartTime > 0) {
            uint64_t elapsed = now - state->flushAnimationStartTime;
            
            if (elapsed >= CombatEffects::DAMAGE_ACCUMULATOR_FADE_MS) {
                // Animation finished: Reset everything for the next burst.
                state->accumulatedDamage = 0.0f;
                state->flushAnimationStartTime = 0;
                return; // Don't draw anything this frame.
            } else {
                // We are mid-fade. Calculate the alpha.
                float progress = static_cast<float>(elapsed) / CombatEffects::DAMAGE_ACCUMULATOR_FADE_MS;
                animationAlpha = 1.0f - Animation::EaseOutCubic(progress); // Fade from 1.0 to 0.0
            }
        }

        float realHealth = entity->currentHealth;
        float endHealth = realHealth + state->accumulatedDamage;

        float startPercent = realHealth / entity->maxHealth;
        float endPercent = endHealth / entity->maxHealth;
        if (endPercent > 1.f) endPercent = 1.f;
        if (endPercent <= startPercent) return;

        ImVec2 oMin(barMin.x + barWidth * startPercent, barMin.y);
        ImVec2 oMax(barMin.x + barWidth * endPercent, barMin.y + barHeight);

        ImU32 base = IM_COL32(255, 255, 150, 150);
        unsigned int a = (base >> 24) & 0xFF;
        
        // Apply BOTH the distance fade AND our new animation fade.
        unsigned int finalA = static_cast<unsigned int>(a * fadeAlpha * animationAlpha + 0.5f);
        base = (base & 0x00FFFFFF) | (ClampAlpha(finalA) << 24);

        DrawFilledRect(dl, oMin, oMax, base, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawDamageFlash(ImDrawList* dl,
        const EntityCombatState* state,
        const RenderableEntity* entity,
        uint64_t now,
        const ImVec2& barMin,
        float barWidth,
        float barHeight,
        float fadeAlpha) {
        if (!state || !entity || entity->maxHealth <= 0) return;
        if (state->lastHitTimestamp == 0) return;

        uint64_t elapsed = now - state->lastHitTimestamp;
        if (elapsed >= CombatEffects::DAMAGE_FLASH_TOTAL_DURATION_MS) return;

        float flashAlpha = 1.0f;
        if (elapsed > CombatEffects::DAMAGE_FLASH_HOLD_DURATION_MS) {
            uint64_t intoFade = elapsed - CombatEffects::DAMAGE_FLASH_HOLD_DURATION_MS;
            float fadeProgress = static_cast<float>(intoFade) / CombatEffects::DAMAGE_FLASH_FADE_DURATION_MS;
            if (fadeProgress > 1.f) fadeProgress = 1.f;
            flashAlpha = 1.0f - Animation::EaseOutCubic(fadeProgress);
        }

        float currentPercent = entity->currentHealth / entity->maxHealth;
        float previousPercent = (entity->currentHealth + state->lastDamageTaken) / entity->maxHealth;
        if (previousPercent > 1.f) previousPercent = 1.f;
        if (previousPercent <= currentPercent) return;

        ImVec2 fMin(barMin.x + barWidth * currentPercent, barMin.y);
        ImVec2 fMax(barMin.x + barWidth * previousPercent, barMin.y + barHeight);

        ImU32 flashColor = IM_COL32(255, 255, 0, static_cast<int>(flashAlpha * 255 * fadeAlpha));
        DrawFilledRect(dl, fMin, fMax, flashColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::DrawBarrierOverlay(ImDrawList* dl,
        const RenderableEntity* entity,
        const EntityCombatState* state,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        float barHeight,
        float fadeAlpha)
    {
        if (!entity || entity->maxHealth <= 0) return;

        const float currentBarrier = entity->currentBarrier;
        float animatedBarrier = currentBarrier;

        // Keep your easing for a smooth change
        if (state && state->lastBarrierChangeTimestamp > 0) {
            const uint64_t now = NowMs();
            const uint64_t elapsed = now - state->lastBarrierChangeTimestamp;
            if (elapsed < CombatEffects::BARRIER_ANIM_DURATION_MS) {
                const float progress = static_cast<float>(elapsed) / CombatEffects::BARRIER_ANIM_DURATION_MS;
                const float eased = Animation::EaseOutCubic(progress);
                animatedBarrier = state->barrierOnLastChange +
                    (currentBarrier - state->barrierOnLastChange) * eased;
            }
        }

        if (animatedBarrier <= 0.0f) return;

        const float healthPercent = entity->currentHealth / entity->maxHealth;
        const float barrierPercent = animatedBarrier / entity->maxHealth;

        static constexpr ImU32 BARRIER_COLOR = IM_COL32(255, 230, 180, 240); // single barrier color
        static constexpr ImU32 OVERFLOW_OUTLINE_COLOR = IM_COL32(255, 255, 255, 210); // separator on full bars

        const ImU32 barrierColor = ApplyAlphaToColor(BARRIER_COLOR, fadeAlpha);
        const ImU32 overflowOutlineColor = ApplyAlphaToColor(OVERFLOW_OUTLINE_COLOR, fadeAlpha);

        // 1) Barrier inside the remaining health segment, left to right
        if (healthPercent < 1.0f) {
            const float startP = healthPercent;
            const float endP = (std::min)(1.0f, healthPercent + barrierPercent);
            if (endP > startP) {
                ImVec2 fillP0(barMin.x + barWidth * startP, barMin.y);
                ImVec2 fillP1(barMin.x + barWidth * endP, barMax.y);
                DrawFilledRect(dl, fillP0, fillP1, barrierColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
            }
        }

        // 2) Barrier overflow, anchored to the right edge when clamped at full
        if (healthPercent + barrierPercent > 1.0f) {
            const float overflowAmount = (healthPercent + barrierPercent) - 1.0f;
            if (overflowAmount > 0.0f) {
                const float ow = barWidth * (std::min)(1.0f, overflowAmount);
                ImVec2 ovrP0(barMax.x - ow, barMin.y);
                ImVec2 ovrP1(barMax.x, barMax.y);

                DrawFilledRect(dl, ovrP0, ovrP1, barrierColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

                // Outline and separator so it stays readable on a full bar
                dl->AddRect(ovrP0, ovrP1, overflowOutlineColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING, 0, 1.0f);
                dl->AddLine(ImVec2(ovrP0.x, barMin.y), ImVec2(ovrP0.x, barMax.y),
                    ApplyAlphaToColor(IM_COL32(255, 255, 255, 180), fadeAlpha), 1.0f);
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------------
    void ESPHealthBarRenderer::RenderStandaloneHealthBar(ImDrawList* drawList,
        const glm::vec2& centerPos,
        const EntityRenderContext& context,
        unsigned int entityColor,
        float barWidth,
        float barHeight,
        CombatStateManager& stateManager) { // Removed const
        if (context.healthPercent < -1.0f) return; // allow exactly 0 for dead
        const RenderableEntity* entity = context.entity;

        EntityCombatState* state = stateManager.GetStateNonConst(entity ? entity->address : nullptr);
        uint64_t now = NowMs();

        float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;
        float timeFade = 1.0f;

        // Death fade-out
        if (state && state->deathTimestamp > 0) {
            uint64_t sinceDeath = now - state->deathTimestamp;
            if (sinceDeath > CombatEffects::DEATH_BURST_DURATION_MS) {
                uint64_t intoFade = sinceDeath - CombatEffects::DEATH_BURST_DURATION_MS;
                if (intoFade < CombatEffects::DEATH_FINAL_FADE_DURATION_MS) {
                    timeFade = 1.0f - static_cast<float>(intoFade) / CombatEffects::DEATH_FINAL_FADE_DURATION_MS;
                }
                else {
                    timeFade = 0.0f;
                }
            }
        }
        if (timeFade <= 0.f) return;
        fadeAlpha *= timeFade;

        // Geometry
        const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET;
        ImVec2 barMin(centerPos.x - barWidth * 0.5f, centerPos.y + yOffset);
        ImVec2 barMax(centerPos.x + barWidth * 0.5f, barMin.y + barHeight);

        // Background
        unsigned int bgAlpha =
            static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha + 0.5f);
        drawList->AddRectFilled(barMin,
            barMax,
            IM_COL32(0, 0, 0, ClampAlpha(bgAlpha)),
            RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

        // Alive vs Dead specialized rendering
        if (state && state->deathTimestamp > 0) {
            RenderDeadState(drawList, state, barMin, barMax, barWidth, fadeAlpha);
        }
        else {
            RenderAliveState(drawList, context, state, barMin, barMax, barWidth, entityColor, fadeAlpha);
        }

        // Border (hostile only)
        if (context.attitude == Game::Attitude::Hostile) {
            unsigned int borderAlpha =
                static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha + 0.5f);
            drawList->AddRect(barMin,
                barMax,
                IM_COL32(0, 0, 0, ClampAlpha(borderAlpha)),
                RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING,
                0,
                RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS);
        }
    }

    void ESPHealthBarRenderer::UpdateAccumulatedDamageAnimation(EntityCombatState* state,
                                                              const RenderableEntity* entity,
                                                              uint64_t now,
                                                              float barWidth) {
        if (!state || !entity || state->accumulatedDamage <= 0 || state->flushAnimationStartTime > 0) {
            return;
        }

        bool shouldFlush = false;

        // 1. Calculate the dynamic percentage threshold.
        float hp = (std::max)(1.0f, entity->maxHealth);
        float thresholdPercent = CombatEffects::DESIRED_CHUNK_PIXELS / barWidth;
        float hpLog = log10f(hp);
        float scaleFactor = std::clamp(1.0f - (hpLog - 4.0f) * 0.15f, 0.25f, 1.3f);
        thresholdPercent *= scaleFactor;
        thresholdPercent = std::clamp(thresholdPercent,
                                      CombatEffects::MIN_CHUNK_PERCENT,
                                      CombatEffects::MAX_CHUNK_PERCENT);

        // 2. PRIMARY CONDITION: Flush if the chunk has reached a satisfying size.
        float accumulatedPercent = state->accumulatedDamage / hp;
        if (accumulatedPercent >= thresholdPercent) {
            shouldFlush = true;
        }
        // 3. FALLBACK CONDITION: Flush if the burst has ended.
        else if (now - state->lastHitTimestamp > CombatEffects::BURST_INACTIVITY_TIMEOUT_MS) {
            shouldFlush = true;
        }

        if (shouldFlush) {
            state->flushAnimationStartTime = now;
        }
    }

    void ESPHealthBarRenderer::RenderAliveState(ImDrawList* drawList,
        const EntityRenderContext& context,
        EntityCombatState* state,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        unsigned int entityColor,
        float fadeAlpha) {
        const RenderableEntity* entity = context.entity;
        if (!entity || entity->maxHealth <= 0) return;

        uint64_t now = NowMs();
        float barHeight = barMax.y - barMin.y;

        // Decide if we should start the damage accumulator fade-out animation.
        UpdateAccumulatedDamageAnimation(state, entity, now, barWidth);

        // 1. Base health fill
        DrawHealthBase(drawList, barMin, barMax, barWidth, context.healthPercent, entityColor, fadeAlpha);

        if (!state) return;

        // 2. Healing overlays
        DrawHealOverlay(drawList, state, entity, now, barMin, barWidth, barHeight);
        DrawHealFlash(drawList, state, entity, now, barMin, barWidth, barHeight, fadeAlpha);

        // 3. Accumulated damage
        DrawAccumulatedDamage(drawList, state, entity, barMin, barWidth, barHeight, fadeAlpha);

        // 4. Damage flash
        DrawDamageFlash(drawList, state, entity, now, barMin, barWidth, barHeight, fadeAlpha);

        // 5. Barrier overlay (drawn last, on top of everything)
        DrawBarrierOverlay(drawList, entity, state, barMin, barMax, barWidth, barHeight, fadeAlpha);
    }

    void ESPHealthBarRenderer::RenderDeadState(ImDrawList* drawList,
        const EntityCombatState* state,
        const ImVec2& barMin,
        const ImVec2& barMax,
        float barWidth,
        float fadeAlpha) {
        if (!state) return;
        uint64_t now = NowMs();
        uint64_t sinceDeath = now - state->deathTimestamp;

        if (sinceDeath >= CombatEffects::DEATH_BURST_DURATION_MS) return;

        float linear = static_cast<float>(sinceDeath) / CombatEffects::DEATH_BURST_DURATION_MS;
        if (linear > 1.f) linear = 1.f;

        float eased = Animation::EaseOutCubic(linear);
        float burstAlpha = 1.0f - Animation::EaseOutCubic(linear); // Eased alpha fade

        // Invert the animation: start wide and shrink to center for an "impact" feel.
        float width = barWidth * (1.0f - eased);
        ImVec2 center(barMin.x + barWidth * 0.5f, (barMin.y + barMax.y) * 0.5f);
        ImVec2 burstMin(center.x - width * 0.5f, barMin.y);
        ImVec2 burstMax(center.x + width * 0.5f, barMax.y);

        ImU32 burstColor = IM_COL32(200, 255, 255, static_cast<int>(burstAlpha * 255 * fadeAlpha));
        DrawFilledRect(drawList, burstMin, burstMax, burstColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

    void ESPHealthBarRenderer::RenderStandaloneEnergyBar(ImDrawList* drawList,
        const glm::vec2& centerPos,
        float energyPercent,
        float fadeAlpha,
        float barWidth,
        float barHeight,
        float healthBarHeight) {
        if (energyPercent < 0.0f || energyPercent > 1.0f) return;

        const float yOffset =
            RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET + healthBarHeight + 2.0f; // 2px gap below health bar
        ImVec2 barMin(centerPos.x - barWidth * 0.5f, centerPos.y + yOffset);
        ImVec2 barMax(centerPos.x + barWidth * 0.5f, barMin.y + barHeight);

        // Background
        unsigned int bgAlpha =
            ClampAlpha(static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha + 0.5f));
        drawList->AddRectFilled(barMin,
            barMax,
            IM_COL32(0, 0, 0, bgAlpha),
            RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

        // Energy fill
        float fillWidth = barWidth * energyPercent;
        ImVec2 eMin(barMin.x, barMin.y);
        ImVec2 eMax(barMin.x + fillWidth, barMax.y);

        ImU32 energyColor = ESPColors::ENERGY_BAR;
        float colorA = ((energyColor >> 24) & 0xFF) / 255.0f;
        ImU32 finalColor = ApplyAlphaToColor(energyColor, colorA * fadeAlpha);

        DrawFilledRect(drawList, eMin, eMax, finalColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }

} // namespace kx