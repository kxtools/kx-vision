#include "HealthBarAnimations.h"

#include "../Data/RenderableData.h"
#include "../Combat/CombatState.h"
#include "../Data/EntityRenderContext.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/AnimationHelpers.h"

namespace kx {

namespace {

// Handles the overall fade-out of the bar on entity death.
void AnimateOverallFade(const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    float timeFade = 1.0f;
    if (state->deathTimestamp > 0) {
        uint64_t sinceDeath = now - state->deathTimestamp;
        if (sinceDeath > CombatEffects::DEATH_BURST_DURATION_MS) {
            uint64_t intoFade = sinceDeath - CombatEffects::DEATH_BURST_DURATION_MS;
            if (intoFade < CombatEffects::DEATH_FINAL_FADE_DURATION_MS) {
                timeFade = 1.0f - static_cast<float>(intoFade) / CombatEffects::DEATH_FINAL_FADE_DURATION_MS;
            } else {
                timeFade = 0.0f;
            }
        }
    }
    animState.healthBarFadeAlpha = timeFade;
}

void AnimateDeathBurst(const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->deathTimestamp > 0)
    {
        uint64_t sinceDeath = now - state->deathTimestamp;
        if (sinceDeath < CombatEffects::DEATH_BURST_DURATION_MS)
        {
            float linear = static_cast<float>(sinceDeath) / CombatEffects::DEATH_BURST_DURATION_MS;
            float eased = Animation::EaseOutCubic(linear);
            animState.deathBurstAlpha = 1.0f - eased;
            animState.deathBurstWidth = 1.0f - eased;
        }
    }
}

void AnimateDamageAccumulator(const RenderableEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->accumulatedDamage > 0)
    {
        // Default to fully opaque
        animState.damageAccumulatorAlpha = 1.0f;

        // If a flush animation is running, calculate the fade-out alpha for the bar and number
        if (state->flushAnimationStartTime > 0)
        {
            uint64_t elapsed = now - state->flushAnimationStartTime;
            if (elapsed < CombatEffects::DAMAGE_ACCUMULATOR_FADE_MS)
            {
                float progress = static_cast<float>(elapsed) / CombatEffects::DAMAGE_ACCUMULATOR_FADE_MS;
                
                // Animate bar alpha
                animState.damageAccumulatorAlpha = 1.0f - Animation::EaseOutCubic(progress);

                // Animate the damage number
                float easedProgress = Animation::EaseOutCubic(progress);
                animState.damageNumberToDisplay = state->damageToDisplay;
                animState.damageNumberAlpha = 1.0f - easedProgress; // Linear fade out for text
                animState.damageNumberYOffset = easedProgress * CombatEffects::DAMAGE_NUMBER_MAX_Y_OFFSET; // Scroll up
            }
            else
            {
                // Animation is over, render at zero alpha this frame before it's reset
                animState.damageAccumulatorAlpha = 0.0f;
                animState.damageNumberAlpha = 0.0f;
            }
        }

        // The size of the chunk is based on the health values
        if (entity->maxHealth > 0) {
            float endHealth = entity->currentHealth + state->accumulatedDamage;
            animState.damageAccumulatorPercent = endHealth / entity->maxHealth;
        }
    }
}

void AnimateHealOverlay(const RenderableEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->lastHealTimestamp > 0) {
        uint64_t elapsed = now - state->lastHealTimestamp;
        if (elapsed < CombatEffects::HEAL_OVERLAY_DURATION_MS) {
            animState.healOverlayStartPercent = state->healStartHealth / entity->maxHealth;
            animState.healOverlayEndPercent = entity->currentHealth / entity->maxHealth;

            float overlayAlpha = 1.0f;
            if (CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS < CombatEffects::HEAL_OVERLAY_DURATION_MS) {
                uint64_t fadeStart = CombatEffects::HEAL_OVERLAY_DURATION_MS - CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;
                if (elapsed > fadeStart) {
                    uint64_t intoFade = elapsed - fadeStart;
                    float fadeProgress = static_cast<float>(intoFade) / CombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;
                    overlayAlpha = 1.0f - Animation::EaseOutCubic(fadeProgress);
                }
            }
            animState.healOverlayAlpha = overlayAlpha;
        }
    }
}

void AnimateDamageFlash(const RenderableEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->lastHitTimestamp > 0) {
        uint64_t elapsed = now - state->lastHitTimestamp;
        if (elapsed < CombatEffects::DAMAGE_FLASH_TOTAL_DURATION_MS) {
            float flashAlpha = 1.0f;
            if (elapsed > CombatEffects::DAMAGE_FLASH_HOLD_DURATION_MS) {
                uint64_t intoFade = elapsed - CombatEffects::DAMAGE_FLASH_HOLD_DURATION_MS;
                float fadeProgress = static_cast<float>(intoFade) / CombatEffects::DAMAGE_FLASH_FADE_DURATION_MS;
                flashAlpha = 1.0f - Animation::EaseOutCubic(fadeProgress);
            }
            animState.damageFlashAlpha = flashAlpha;
            animState.damageFlashStartPercent = (entity->currentHealth + state->lastDamageTaken) / entity->maxHealth;
        }
    }
}

void AnimateHealFlash(const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->lastHealFlashTimestamp > 0) {
        uint64_t elapsed = now - state->lastHealFlashTimestamp;
        if (elapsed < CombatEffects::HEAL_FLASH_DURATION_MS) {
            float linear = static_cast<float>(elapsed) / CombatEffects::HEAL_FLASH_DURATION_MS;
            animState.healFlashAlpha = 1.0f - Animation::EaseOutCubic(linear);
        }
    }
}

void AnimateLivingEffects(const RenderableEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->deathTimestamp == 0 && entity->maxHealth > 0) {
        AnimateHealOverlay(entity, state, animState, now);
        AnimateDamageFlash(entity, state, animState, now);
        AnimateHealFlash(state, animState, now);
    }
}

void AnimateBarrier(const RenderableEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    const float currentBarrier = entity->currentBarrier;
    float animatedBarrier = currentBarrier;
    if (state->lastBarrierChangeTimestamp > 0) {
        const uint64_t elapsed = now - state->lastBarrierChangeTimestamp;
        if (elapsed < CombatEffects::BARRIER_ANIM_DURATION_MS) {
            const float progress = static_cast<float>(elapsed) / CombatEffects::BARRIER_ANIM_DURATION_MS;
            const float eased = Animation::EaseOutCubic(progress);
            animatedBarrier = state->barrierOnLastChange + (currentBarrier - state->barrierOnLastChange) * eased;
        }
    }
    animState.animatedBarrier = animatedBarrier;
}

} // anonymous namespace

void PopulateHealthBarAnimations(const RenderableEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (!entity || !state) {
        return;
    }

    AnimateOverallFade(state, animState, now);
    if (animState.healthBarFadeAlpha <= 0.0f) {
        return; // No need to calculate other effects if the bar is invisible
    }

    AnimateDeathBurst(state, animState, now);
    AnimateDamageAccumulator(entity, state, animState, now);
    AnimateLivingEffects(entity, state, animState, now);
    AnimateBarrier(entity, state, animState, now);
}

} // namespace kx
