#include "HealthBarAnimations.h"

#include "../../../../Game/Data/EntityData.h"
#include "../../../../Game/Services/Combat/CombatObservation.h"
#include "../../Combat/VisualCombatState.h"
#include "../../Combat/VisualCombatConstants.h"
#include "../../../../Rendering/Data/HealthBarAnimationState.h"
#include "../../../../Rendering/Shared/AnimationHelpers.h"

namespace kx {

namespace {

// Handles the overall fade-out of the bar on entity death.
void AnimateOverallFade(const CombatObservation* state, HealthBarAnimationState& animState, uint64_t now) {
    float timeFade = 1.0f;
    if (state->deathTimestamp > 0) {
        uint64_t sinceDeath = CombatElapsedMs(now, state->deathTimestamp);
        if (sinceDeath > VisualCombatEffects::DEATH_BURST_DURATION_MS) {
            uint64_t intoFade = sinceDeath - VisualCombatEffects::DEATH_BURST_DURATION_MS;
            if (intoFade < VisualCombatEffects::DEATH_FINAL_FADE_DURATION_MS) {
                float progress = static_cast<float>(intoFade) / VisualCombatEffects::DEATH_FINAL_FADE_DURATION_MS;
                timeFade = 1.0f - Animation::EaseOutCubic(progress);
            } else {
                timeFade = 0.0f;
            }
        }
    }
    animState.healthBarFadeAlpha = timeFade;
}

void AnimateDeathBurst(const CombatObservation* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->deathTimestamp > 0)
    {
        uint64_t sinceDeath = CombatElapsedMs(now, state->deathTimestamp);
        if (sinceDeath < VisualCombatEffects::DEATH_BURST_DURATION_MS)
        {
            float linear = static_cast<float>(sinceDeath) / VisualCombatEffects::DEATH_BURST_DURATION_MS;
            float eased = Animation::EaseOutCubic(linear);
            animState.deathBurstAlpha = 1.0f - eased;
            animState.deathBurstWidth = 1.0f - eased;
        }
    }
}

void AnimateDamageAccumulator(const GameEntity* entity, const VisualCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->accumulatedDamage > 0)
    {
        // Default to fully opaque
        animState.damageAccumulatorAlpha = 1.0f;

        // If a flush animation is running, calculate the fade-out alpha for the bar and number
        if (state->flushAnimationStartTime > 0)
        {
            uint64_t elapsed = CombatElapsedMs(now, state->flushAnimationStartTime);
            if (elapsed < VisualCombatEffects::DAMAGE_ACCUMULATOR_FADE_MS)
            {
                float progress = static_cast<float>(elapsed) / VisualCombatEffects::DAMAGE_ACCUMULATOR_FADE_MS;
                
                // Animate bar alpha
                animState.damageAccumulatorAlpha = 1.0f - Animation::EaseOutCubic(progress);

                // Animate the damage number
                float easedProgress = Animation::EaseOutCubic(progress);
                animState.damageNumberToDisplay = state->damageToDisplay;
                animState.damageNumberAlpha = 1.0f - easedProgress; // Linear fade out for text
                animState.damageNumberYOffset = easedProgress * VisualCombatEffects::DAMAGE_NUMBER_MAX_Y_OFFSET; // Scroll up
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

void AnimateHealOverlay(const GameEntity* entity, const CombatObservation* observation, const VisualCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (observation->lastHealTimestamp > 0) {
        uint64_t elapsed = CombatElapsedMs(now, observation->lastHealTimestamp);
        if (elapsed < VisualCombatEffects::HEAL_OVERLAY_DURATION_MS) {
            animState.healOverlayStartPercent = state->healStartHealth / entity->maxHealth;
            animState.healOverlayEndPercent = entity->currentHealth / entity->maxHealth;

            float overlayAlpha = 1.0f;
            if (VisualCombatEffects::HEAL_OVERLAY_FADE_DURATION_MS < VisualCombatEffects::HEAL_OVERLAY_DURATION_MS) {
                uint64_t fadeStart = VisualCombatEffects::HEAL_OVERLAY_DURATION_MS - VisualCombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;
                if (elapsed > fadeStart) {
                    uint64_t intoFade = elapsed - fadeStart;
                    float fadeProgress = static_cast<float>(intoFade) / VisualCombatEffects::HEAL_OVERLAY_FADE_DURATION_MS;
                    overlayAlpha = 1.0f - Animation::EaseOutCubic(fadeProgress);
                }
            }
            animState.healOverlayAlpha = overlayAlpha;
        }
    }
}

void AnimateDamageFlash(const GameEntity* entity, const CombatObservation* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->lastHitTimestamp > 0) {
        uint64_t elapsed = CombatElapsedMs(now, state->lastHitTimestamp);
        if (elapsed < VisualCombatEffects::DAMAGE_FLASH_TOTAL_DURATION_MS) {
            float flashAlpha = 1.0f;
            if (elapsed > VisualCombatEffects::DAMAGE_FLASH_HOLD_DURATION_MS) {
                uint64_t intoFade = elapsed - VisualCombatEffects::DAMAGE_FLASH_HOLD_DURATION_MS;
                float fadeProgress = static_cast<float>(intoFade) / VisualCombatEffects::DAMAGE_FLASH_FADE_DURATION_MS;
                flashAlpha = 1.0f - Animation::EaseOutCubic(fadeProgress);
            }
            animState.damageFlashAlpha = flashAlpha;
            animState.damageFlashStartPercent = (entity->currentHealth + state->lastDamageTaken) / entity->maxHealth;
        }
    }
}

void AnimateHealFlash(const CombatObservation* state, HealthBarAnimationState& animState, uint64_t now) {
    if (state->lastHealTimestamp > 0) {
        uint64_t elapsed = CombatElapsedMs(now, state->lastHealTimestamp);
        if (elapsed < VisualCombatEffects::HEAL_FLASH_DURATION_MS) {
            float linear = static_cast<float>(elapsed) / VisualCombatEffects::HEAL_FLASH_DURATION_MS;
            animState.healFlashAlpha = 1.0f - Animation::EaseOutCubic(linear);
        }
    }
}

void AnimateLivingEffects(const GameEntity* entity, const CombatObservation* observation, const VisualCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (observation->deathTimestamp == 0 && entity->maxHealth > 0) {
        AnimateHealOverlay(entity, observation, state, animState, now);
        AnimateDamageFlash(entity, observation, animState, now);
        AnimateHealFlash(observation, animState, now);
    }
}

void AnimateBarrier(const GameEntity* entity, const CombatObservation* state, HealthBarAnimationState& animState, uint64_t now) {
    const float currentBarrier = entity->currentBarrier;
    float animatedBarrier = currentBarrier;
    if (state->lastBarrierChangeTimestamp > 0) {
        const uint64_t elapsed = CombatElapsedMs(now, state->lastBarrierChangeTimestamp);
        if (elapsed < VisualCombatEffects::BARRIER_ANIM_DURATION_MS) {
            const float progress = static_cast<float>(elapsed) / VisualCombatEffects::BARRIER_ANIM_DURATION_MS;
            const float eased = Animation::EaseOutCubic(progress);
            animatedBarrier = state->barrierOnLastChange + (currentBarrier - state->barrierOnLastChange) * eased;
        }
    }
    animState.animatedBarrier = animatedBarrier;
}

} // anonymous namespace

void PopulateHealthBarAnimations(const GameEntity* entity, const CombatObservation* observation, const VisualCombatState* visualState, HealthBarAnimationState& animState, uint64_t now) {
    if (!entity || !observation || !visualState) {
        return;
    }

    AnimateOverallFade(observation, animState, now);
    if (animState.healthBarFadeAlpha <= 0.0f) {
        return; // No need to calculate other effects if the bar is invisible
    }

    AnimateDeathBurst(observation, animState, now);
    AnimateDamageAccumulator(entity, visualState, animState, now);
    AnimateLivingEffects(entity, observation, visualState, animState, now);
    AnimateBarrier(entity, observation, animState, now);
}

} // namespace kx
