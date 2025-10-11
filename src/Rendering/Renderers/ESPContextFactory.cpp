#include "ESPContextFactory.h"

#include <Windows.h> // For GetTickCount64
#include "../Combat/CombatStateManager.h" // For CombatStateManager
#include "../Utils/ESPConstants.h" // For CombatEffects
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

    // 2. Death Burst Animation
    if (state->deathTimestamp > 0) {
        uint64_t sinceDeath = now - state->deathTimestamp;
        if (sinceDeath < CombatEffects::DEATH_BURST_DURATION_MS) {
            float linear = static_cast<float>(sinceDeath) / CombatEffects::DEATH_BURST_DURATION_MS;
            float eased = Animation::EaseOutCubic(linear);
            animState.deathBurstAlpha = 1.0f - eased;
            animState.deathBurstWidth = 1.0f - eased;
        }
    } else {
        // Only calculate these effects if the entity is alive
        if (maxHealth > 0) {
            // 3. Damage Accumulator
            if (state->accumulatedDamage > 0) {
                // Default to fully opaque
                animState.damageAccumulatorAlpha = 1.0f;

                // If a flush animation is running, calculate the fade-out alpha
                if (state->flushAnimationStartTime > 0) {
                    uint64_t elapsed = now - state->flushAnimationStartTime;
                    if (elapsed < CombatEffects::DAMAGE_ACCUMULATOR_FADE_MS) {
                        float progress = static_cast<float>(elapsed) / CombatEffects::DAMAGE_ACCUMULATOR_FADE_MS;
                        animState.damageAccumulatorAlpha = 1.0f - Animation::EaseOutCubic(progress); // Animate alpha from 1.0 to 0.0
                    } else {
                        // Animation is over, render at zero alpha this frame before it's reset
                        animState.damageAccumulatorAlpha = 0.0f;
                    }
                }

                // The size of the chunk is based on the health values
                float endHealth = entity->currentHealth + state->accumulatedDamage;
                animState.damageAccumulatorPercent = endHealth / maxHealth;
            }

            // 4. Heal Overlay
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

            // 5. Damage Flash
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

            // 6. Heal Flash
            if (state->lastHealFlashTimestamp > 0) {
                uint64_t elapsed = now - state->lastHealFlashTimestamp;
                if (elapsed < CombatEffects::HEAL_FLASH_DURATION_MS) {
                    float linear = static_cast<float>(elapsed) / CombatEffects::HEAL_FLASH_DURATION_MS;
                    animState.healFlashAlpha = 1.0f - Animation::EaseOutCubic(linear);
                }
            }
        }
    }
	
    // 7. Barrier Animation (always runs, even if dead, to animate to zero)
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

EntityRenderContext ESPContextFactory::CreateContextForPlayer(const RenderablePlayer* player,
                                                             const Settings& settings,
                                                             const CombatStateManager& stateManager,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight,
                                                             uint64_t now) { // Added 'now'
    float healthPercent = (player->maxHealth > 0) ? (player->currentHealth / player->maxHealth) : -1.0f;
    float energyPercent = -1.0f;
    if (settings.playerESP.energyDisplayType == EnergyDisplayType::Dodge) {
        if (player->maxEnergy > 0) {
            energyPercent = player->currentEnergy / player->maxEnergy;
        }
    } else { // Special
        if (player->maxSpecialEnergy > 0) {
            energyPercent = player->currentSpecialEnergy / player->maxSpecialEnergy;
        }
    }
    
    // Use attitude-based coloring for players (same as NPCs for semantic consistency)
    unsigned int color;
    switch (player->attitude) {
        case Game::Attitude::Hostile:
            color = ESPColors::NPC_HOSTILE;
            break;
        case Game::Attitude::Friendly:
            color = ESPColors::NPC_FRIENDLY;
            break;
        case Game::Attitude::Neutral:
            color = ESPColors::NPC_NEUTRAL;
            break;
        case Game::Attitude::Indifferent:
            color = ESPColors::NPC_INDIFFERENT;
            break;
        default:
            color = ESPColors::NPC_UNKNOWN;
            break;
    }

    // --- Animation State --- 
    const EntityCombatState* state = stateManager.GetState(player->address);
    HealthBarAnimationState animState;
    if (state) {
        PopulateHealthBarAnimations(player, state, animState, now); // Pass 'now' to the animation logic
    }
    
    return EntityRenderContext{
        player->position,
        player->visualDistance,
        player->gameplayDistance,
        color,
        details,
        healthPercent,
        energyPercent,
        settings.playerESP.renderBox,
        settings.playerESP.renderDistance,
        settings.playerESP.renderDot,
        !details.empty(),
        settings.playerESP.renderHealthBar,
        settings.playerESP.renderEnergyBar,
        settings.playerESP.renderPlayerName,
        ESPEntityType::Player,
        player->attitude,
        Game::CharacterRank::Ambient, // Players are considered Ambient for rank purposes
        screenWidth,
        screenHeight,
        player, // entity pointer
        player->playerName,
        player,
        animState
    };
}

EntityRenderContext ESPContextFactory::CreateContextForNpc(const RenderableNpc* npc,
                                                          const Settings& settings,
                                                          const CombatStateManager& stateManager,
                                                          const std::vector<ColoredDetail>& details,
                                                          float screenWidth,
                                                          float screenHeight,
                                                          uint64_t now) { // Added 'now'
    float healthPercent = (npc->maxHealth > 0) ? (npc->currentHealth / npc->maxHealth) : -1.0f;
    
    // Use attitude-based coloring for NPCs
    unsigned int color;
    switch (npc->attitude) {
        case Game::Attitude::Hostile:
            color = ESPColors::NPC_HOSTILE;
            break;
        case Game::Attitude::Friendly:
            color = ESPColors::NPC_FRIENDLY;
            break;
        case Game::Attitude::Neutral:
            color = ESPColors::NPC_NEUTRAL;
            break;
        case Game::Attitude::Indifferent:
            color = ESPColors::NPC_INDIFFERENT;
            break;
        default:
            color = ESPColors::NPC_UNKNOWN;
            break;
    }

    // --- Animation State --- 
    const EntityCombatState* state = stateManager.GetState(npc->address);
    HealthBarAnimationState animState;
    if (state) {
        PopulateHealthBarAnimations(npc, state, animState, now); // Pass 'now' to the animation logic
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
        settings.npcESP.renderBox,
        settings.npcESP.renderDistance,
        settings.npcESP.renderDot,
        settings.npcESP.renderDetails,
        settings.npcESP.renderHealthBar,
        false, // No energy bar for NPCs
        false,
        ESPEntityType::NPC,
        npc->attitude,
        npc->rank,
        screenWidth,
        screenHeight,
        npc, // entity pointer
        emptyPlayerName,
        nullptr,
        animState
    };
}

EntityRenderContext ESPContextFactory::CreateContextForGadget(const RenderableGadget* gadget,
                                                             const Settings& settings,
                                                             const CombatStateManager& stateManager,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight,
                                                             uint64_t now) { // Added 'now'
    static const std::string emptyPlayerName = "";

    bool renderHealthBar = settings.objectESP.renderHealthBar;

    // Do not render health bar for certain gadget types to avoid flickering, or if the base setting is off.
    if (renderHealthBar) {
        if (ESPStyling::ShouldHideHealthBarForGadgetType(gadget->type)) {
            renderHealthBar = false;
        }
        else if (gadget->maxHealth > 0) {
            // "Only show damaged" filter: Hide bar if gadget is at full health.
            if (settings.objectESP.showOnlyDamagedGadgets && gadget->currentHealth >= gadget->maxHealth) {
                renderHealthBar = false;
            }
            // "Don't show bar on already-dead gadgets" filter: Hide bar if gadget is dead and animation is over.
            else if (gadget->currentHealth <= 0.0f) {
                const EntityCombatState* state = stateManager.GetState(gadget->address);
                if (!state || state->deathTimestamp == 0 || (now - state->deathTimestamp) > CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS) {
                    renderHealthBar = false;
                }
            }
        }
    }

    // --- Animation State --- 
    HealthBarAnimationState animState;
    if (renderHealthBar) { // Only calculate animations if the bar will be visible
        const EntityCombatState* state = stateManager.GetState(gadget->address);
        if (state) {
            PopulateHealthBarAnimations(gadget, state, animState, now); // Pass 'now' to the animation logic
        }
    }

    return EntityRenderContext{
        gadget->position,
        gadget->visualDistance,
        gadget->gameplayDistance,
        ESPColors::GADGET,
        details,
        gadget->maxHealth > 0 ? (gadget->currentHealth / gadget->maxHealth) : -1.0f,
        -1.0f, // No energy for gadgets
        (settings.objectESP.renderCircle || settings.objectESP.renderSphere),
        settings.objectESP.renderDistance,
        settings.objectESP.renderDot,
        settings.objectESP.renderDetails,
        renderHealthBar,
        false, // No energy bar for gadgets
        false,
        ESPEntityType::Gadget,
        Game::Attitude::Neutral,
        Game::CharacterRank::Normal, // Gadgets don't have ranks
        screenWidth,
        screenHeight,
        gadget, // entity pointer
        emptyPlayerName,
        nullptr,
        animState
    };
}

} // namespace kx