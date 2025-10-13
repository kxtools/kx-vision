#include "ESPContextFactory.h"

#include <Windows.h> // For GetTickCount64
#include "../Combat/CombatStateManager.h" // For CombatStateManager
#include "../Utils/ESPConstants.h" // For CombatEffects
#include "../Data/ESPData.h"
#include "../Core/ESPStageRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../../Game/GameEnums.h"
#include "../Utils/AnimationHelpers.h" // For easing functions
#include "../Utils/ESPStyling.h"

namespace kx {

namespace { // Anonymous namespace for helpers

/**
 * @brief Calculates all transient health bar animation states.
 *
 * This centralizes the animation logic that was previously in the renderer.
 * It populates the HealthBarAnimationState struct, which is then passed to the
 * "dumb" renderer.
 *
 * @param entity The entity being rendered.
 * @param state The combat state for the entity.
 * @param animState The output struct to populate with animation values.
 */
void PopulateHealthBarAnimations(const RenderableEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now) {
    if (!entity || !state) {
        return;
    }

    // const uint64_t now = GetTickCount64(); // REMOVED
    const float maxHealth = entity->maxHealth;

    // 1. Overall Bar Fade (Death)
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
    if (animState.healthBarFadeAlpha <= 0.0f) {
        return; // No need to calculate other effects if the bar is invisible
    }

    // 2. Death Burst Animation (runs only if dead)
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

    // 3. Damage Accumulator Animation (NOW INDEPENDENT - runs if alive OR dead)
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
        if (maxHealth > 0) {
            float endHealth = entity->currentHealth + state->accumulatedDamage;
            animState.damageAccumulatorPercent = endHealth / maxHealth;
        }
    }

    // 4. Living-Only Effects (Healing, Flashes)
    if (state->deathTimestamp == 0 && maxHealth > 0)
    {
        // 4a. Heal Overlay
        if (state->lastHealTimestamp > 0) {
            uint64_t elapsed = now - state->lastHealTimestamp;
            if (elapsed < CombatEffects::HEAL_OVERLAY_DURATION_MS) {
                animState.healOverlayStartPercent = state->healStartHealth / maxHealth;
                animState.healOverlayEndPercent = entity->currentHealth / maxHealth;

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

        // 4b. Damage Flash
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
                animState.damageFlashStartPercent = (entity->currentHealth + state->lastDamageTaken) / maxHealth;
            }
        }

        // 4c. Heal Flash
        if (state->lastHealFlashTimestamp > 0) {
            uint64_t elapsed = now - state->lastHealFlashTimestamp;
            if (elapsed < CombatEffects::HEAL_FLASH_DURATION_MS) {
                float linear = static_cast<float>(elapsed) / CombatEffects::HEAL_FLASH_DURATION_MS;
                animState.healFlashAlpha = 1.0f - Animation::EaseOutCubic(linear);
            }
        }
    }
	
    // 5. Barrier Animation (already independent, which is correct)
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

EntityRenderContext ESPContextFactory::CreateContextForPlayer(const RenderablePlayer* player, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    float healthPercent = (player->maxHealth > 0) ? (player->currentHealth / player->maxHealth) : -1.0f;
    float energyPercent = -1.0f;
    if (context.settings.playerESP.energyDisplayType == EnergyDisplayType::Dodge) {
        if (player->maxEnergy > 0) {
            energyPercent = player->currentEnergy / player->maxEnergy;
        }
    } else { // Special
        if (player->maxSpecialEnergy > 0) {
            energyPercent = player->currentSpecialEnergy / player->maxSpecialEnergy;
        }
    }
    
    // Use attitude-based coloring for players (same as NPCs for semantic consistency)
    unsigned int color = ESPStyling::GetEntityColor(*player);

    // --- Animation State --- 
    const EntityCombatState* state = context.stateManager.GetState(player->address);
    HealthBarAnimationState animState;
    if (state) {
        PopulateHealthBarAnimations(player, state, animState, context.now); // Pass 'now' to the animation logic
    }
    
    return EntityRenderContext{
        player->position,
        player->visualDistance,
        player->gameplayDistance,
        color,
        details,
        healthPercent,
        energyPercent,
        context.settings.playerESP.renderBox,
        context.settings.playerESP.renderDistance,
        context.settings.playerESP.renderDot,
        !details.empty(),
        context.settings.playerESP.renderHealthBar,
        context.settings.playerESP.renderEnergyBar,
        context.settings.playerESP.renderPlayerName,
        ESPEntityType::Player,
        player->attitude,
        Game::CharacterRank::Ambient, // Players are considered Ambient for rank purposes
        context.screenWidth,
        context.screenHeight,
        player, // entity pointer
        player->playerName,
        player,
        animState
    };
}

EntityRenderContext ESPContextFactory::CreateContextForNpc(const RenderableNpc* npc, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    float healthPercent = (npc->maxHealth > 0) ? (npc->currentHealth / npc->maxHealth) : -1.0f;
    
    // Use attitude-based coloring for NPCs
    unsigned int color = ESPStyling::GetEntityColor(*npc);

    // --- Animation State --- 
    const EntityCombatState* state = context.stateManager.GetState(npc->address);
    HealthBarAnimationState animState;
    if (state) {
        PopulateHealthBarAnimations(npc, state, animState, context.now); // Pass 'now' to the animation logic
    }
    
    static const std::string emptyPlayerName = "";
    return EntityRenderContext{
        npc->position,
        npc->visualDistance,
        npc->gameplayDistance,
        color,
        details,
        healthPercent,
        -1.0f, // No energy for NPCs
        context.settings.npcESP.renderBox,
        context.settings.npcESP.renderDistance,
        context.settings.npcESP.renderDot,
        context.settings.npcESP.renderDetails,
        context.settings.npcESP.renderHealthBar,
        false, // No energy bar for NPCs
        false,
        ESPEntityType::NPC,
        npc->attitude,
        npc->rank,
        context.screenWidth,
        context.screenHeight,
        npc, // entity pointer
        emptyPlayerName,
        nullptr,
        animState
    };
}

EntityRenderContext ESPContextFactory::CreateContextForGadget(const RenderableGadget* gadget, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    static const std::string emptyPlayerName = "";

    bool renderHealthBar = context.settings.objectESP.renderHealthBar;

    // Do not render health bar for certain gadget types to avoid flickering, or if the base setting is off.
    if (renderHealthBar) {
        if (ESPStyling::ShouldHideHealthBarForGadgetType(gadget->type)) {
            renderHealthBar = false;
        }
        else if (gadget->maxHealth > 0) {
            // "Only show damaged" filter: Hide bar if gadget is at full health.
            if (context.settings.objectESP.showOnlyDamagedGadgets && gadget->currentHealth >= gadget->maxHealth) {
                renderHealthBar = false;
            }
            // "Don't show bar on already-dead gadgets" filter: Hide bar if gadget is dead and animation is over.
            else if (gadget->currentHealth <= 0.0f) {
                const EntityCombatState* state = context.stateManager.GetState(gadget->address);
                if (!state || state->deathTimestamp == 0 || (context.now - state->deathTimestamp) > CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS) {
                    renderHealthBar = false;
                }
            }
        }
    }

    // --- Animation State --- 
    HealthBarAnimationState animState;
    if (renderHealthBar) { // Only calculate animations if the bar will be visible
        const EntityCombatState* state = context.stateManager.GetState(gadget->address);
        if (state) {
            PopulateHealthBarAnimations(gadget, state, animState, context.now); // Pass 'now' to the animation logic
        }
    }

    return EntityRenderContext{
        gadget->position,
        gadget->visualDistance,
        gadget->gameplayDistance,
        ESPStyling::GetEntityColor(*gadget),
        details,
        gadget->maxHealth > 0 ? (gadget->currentHealth / gadget->maxHealth) : -1.0f,
        -1.0f, // No energy for gadgets
        (context.settings.objectESP.renderCircle || context.settings.objectESP.renderSphere),
        context.settings.objectESP.renderDistance,
        context.settings.objectESP.renderDot,
        context.settings.objectESP.renderDetails,
        renderHealthBar,
        false, // No energy bar for gadgets
        false,
        ESPEntityType::Gadget,
        Game::Attitude::Neutral,
        Game::CharacterRank::Normal, // Gadgets don't have ranks
        context.screenWidth,
        context.screenHeight,
        gadget, // entity pointer
        emptyPlayerName,
        nullptr,
        animState
    };
}

} // namespace kx