#include "ESPContextFactory.h"

#include "../Combat/CombatStateManager.h" // For CombatStateManager
#include "../Utils/ESPConstants.h" // For CombatEffects
#include "../Data/ESPData.h"
#include "../Core/ESPStageRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../../Game/GameEnums.h"
#include "../Utils/ESPStyling.h"
#include "../Animations/HealthBarAnimations.h"

namespace kx {

namespace { // Anonymous namespace for helpers

static bool DeterminePlayerHealthBarVisibility(const RenderablePlayer* player, const PlayerEspSettings& settings) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (settings.showOnlyDamaged && player->maxHealth > 0 && player->currentHealth >= player->maxHealth) {
        return false;
    }
    return true;
}

static bool DetermineNpcHealthBarVisibility(const RenderableNpc* npc, const NpcEspSettings& settings) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (settings.showOnlyDamaged && npc->maxHealth > 0 && npc->currentHealth >= npc->maxHealth) {
        return false;
    }
    return true;
}

static bool DetermineGadgetHealthBarVisibility(const RenderableGadget* gadget, const ObjectEspSettings& settings, const EntityCombatState* state, uint64_t now) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (ESPStyling::ShouldHideCombatUIForGadget(gadget->type)) {
        return false;
    }
    if (gadget->maxHealth > 0) {
        if (settings.showOnlyDamaged && gadget->currentHealth >= gadget->maxHealth) {
            return false;
        }
        if (gadget->currentHealth <= 0.0f) {
            if (!state || state->deathTimestamp == 0 || (now - state->deathTimestamp) > CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS) {
                return false;
            }
        }
    }
    return true;
}

float CalculateBurstDps(const EntityCombatState* state, uint64_t now, bool showBurstDpsSetting) {
    float burstDpsValue = 0.0f;
    if (showBurstDpsSetting) {
        if (state && state->burstStartTime > 0 && state->accumulatedDamage > 0.0f) {
            uint64_t durationMs = now - state->burstStartTime;
            if (durationMs > 100) {
                float durationSeconds = static_cast<float>(durationMs) / 1000.0f;
                burstDpsValue = state->accumulatedDamage / durationSeconds;
            }
        }
    }
    return burstDpsValue;
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

    bool renderHealthBar = DeterminePlayerHealthBarVisibility(player, context.settings.playerESP);

    // --- Animation State --- 
    const EntityCombatState* state = context.stateManager.GetState(player->address);
    HealthBarAnimationState animState;
    if (renderHealthBar && state) {
        PopulateHealthBarAnimations(player, state, animState, context.now); // Pass 'now' to the animation logic
    }
    
    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.playerESP.showBurstDps);
    
    return EntityRenderContext{
        player->position,
        player->visualDistance,
        player->gameplayDistance,
        color,
        details,
        healthPercent,
        energyPercent,
        burstDpsValue, // burstDPS
        context.settings.playerESP.renderBox,
        context.settings.playerESP.renderDistance,
        context.settings.playerESP.renderDot,
        !details.empty(),
        renderHealthBar,
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

    bool renderHealthBar = DetermineNpcHealthBarVisibility(npc, context.settings.npcESP);

    // --- Animation State --- 
    const EntityCombatState* state = context.stateManager.GetState(npc->address);
    HealthBarAnimationState animState;
    if (renderHealthBar && state) {
        PopulateHealthBarAnimations(npc, state, animState, context.now); // Pass 'now' to the animation logic
    }
    
    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.npcESP.showBurstDps);
    
    static const std::string emptyPlayerName = "";
    return EntityRenderContext{
        npc->position,
        npc->visualDistance,
        npc->gameplayDistance,
        color,
        details,
        healthPercent,
        -1.0f, // No energy for NPCs
        burstDpsValue, // burstDPS
        context.settings.npcESP.renderBox,
        context.settings.npcESP.renderDistance,
        context.settings.npcESP.renderDot,
        context.settings.npcESP.renderDetails,
        renderHealthBar,
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

    const EntityCombatState* state = context.stateManager.GetState(gadget->address);
    bool renderHealthBar = DetermineGadgetHealthBarVisibility(gadget, context.settings.objectESP, state, context.now);

    // --- Animation State --- 
    HealthBarAnimationState animState;
    if (renderHealthBar) { // Only calculate animations if the bar will be visible
        if (state) {
            PopulateHealthBarAnimations(gadget, state, animState, context.now); // Pass 'now' to the animation logic
        }
    }

    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.objectESP.showBurstDps);

    return EntityRenderContext{
        gadget->position,
        gadget->visualDistance,
        gadget->gameplayDistance,
        ESPStyling::GetEntityColor(*gadget),
        details,
        gadget->maxHealth > 0 ? (gadget->currentHealth / gadget->maxHealth) : -1.0f,
        -1.0f, // No energy for gadgets
        burstDpsValue, // burstDPS
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