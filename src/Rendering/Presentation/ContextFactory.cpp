#include "ContextFactory.h"

#include "../Combat/CombatStateManager.h" // For CombatStateManager
#include "../Data/FrameData.h"
#include "../Data/EntityRenderContext.h"
#include "../../Game/GameEnums.h"
#include "../Logic/Animations/HealthBarAnimations.h"
#include "../Data/EntityTypes.h"
#include "Settings/ESPSettings.h"
#include "InfoBuilder.h"
#include "Styling.h"
#include "Shared/ColorConstants.h"
#include "Combat/CombatConstants.h"

namespace kx {

static thread_local std::vector<ColoredDetail> s_DetailsBuffer;

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
    if (Styling::ShouldHideCombatUIForGadget(gadget->type)) {
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

EntityRenderContext ContextFactory::CreateContextForPlayer(const RenderablePlayer* player, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    // Use attitude-based coloring for players (same as NPCs for semantic consistency)
    unsigned int color = Styling::GetEntityColor(*player);

    bool renderHealthBar = DeterminePlayerHealthBarVisibility(player, context.settings.playerESP);

    // --- Animation State --- 
    const EntityCombatState* state = context.stateManager.GetState(player->GetCombatKey());
    HealthBarAnimationState animState;
    if (renderHealthBar && state) {
        PopulateHealthBarAnimations(player, state, animState, context.now); // Pass 'now' to the animation logic
    }
    
    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.playerESP.showBurstDps);
    
    return EntityRenderContext{
        .position = player->position,
        .gameplayDistance = player->gameplayDistance,
        .color = color,
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = !details.empty(),
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = context.settings.playerESP.renderEnergyBar,
        .entityType = EntityTypes::Player,
        .attitude = player->attitude,
        .entity = player,
        .playerName = player->playerName,
        .healthBarAnim = animState,
        .showCombatUI = true
    };
}

EntityRenderContext ContextFactory::CreateContextForNpc(const RenderableNpc* npc, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    // Use attitude-based coloring for NPCs
    unsigned int color = Styling::GetEntityColor(*npc);

    bool renderHealthBar = DetermineNpcHealthBarVisibility(npc, context.settings.npcESP);

    // --- Animation State --- 
    const EntityCombatState* state = context.stateManager.GetState(npc->GetCombatKey());
    HealthBarAnimationState animState;
    if (renderHealthBar && state) {
        PopulateHealthBarAnimations(npc, state, animState, context.now); // Pass 'now' to the animation logic
    }
    
    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.npcESP.showBurstDps);
    
    static const std::string emptyPlayerName = "";
    return EntityRenderContext{
        .position = npc->position,
        .gameplayDistance = npc->gameplayDistance,
        .color = color,
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = context.settings.npcESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = false, // No energy bar for NPCs
        .entityType = EntityTypes::NPC,
        .attitude = npc->attitude,
        .entity = npc,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .showCombatUI = true
    };
}

EntityRenderContext ContextFactory::CreateContextForGadget(const RenderableGadget* gadget, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    static const std::string emptyPlayerName = "";

    const EntityCombatState* state = context.stateManager.GetState(gadget->GetCombatKey());
    bool renderHealthBar = DetermineGadgetHealthBarVisibility(gadget, context.settings.objectESP, state, context.now);

    // --- Animation State --- 
    HealthBarAnimationState animState;
    if (renderHealthBar) { // Only calculate animations if the bar will be visible
        if (state) {
            PopulateHealthBarAnimations(gadget, state, animState, context.now); // Pass 'now' to the animation logic
        }
    }

    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.objectESP.showBurstDps);

    // Check if combat UI should be hidden for this gadget type
    bool hideCombatUI = Styling::ShouldHideCombatUIForGadget(gadget->type);

    return EntityRenderContext{
        .position = gadget->position,
        .gameplayDistance = gadget->gameplayDistance,
        .color = Styling::GetEntityColor(*gadget),
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = context.settings.objectESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = false, // No energy bar for gadgets
        .entityType = EntityTypes::Gadget,
        .attitude = Game::Attitude::Neutral,
        .entity = gadget,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .showCombatUI = !hideCombatUI
    };
}

EntityRenderContext ContextFactory::CreateContextForAttackTarget(const RenderableAttackTarget* attackTarget, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    static const std::string emptyPlayerName = "";

    const EntityCombatState* state = context.stateManager.GetState(attackTarget->GetCombatKey());
    bool renderHealthBar = false; // Attack targets typically don't have health data

    // --- Animation State --- 
    HealthBarAnimationState animState;
    // Attack targets don't typically have health, so no animation state needed

    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.objectESP.showBurstDps);

    bool hideCombatUI = false; // Can be customized if needed

    unsigned int color = Styling::GetEntityColor(*attackTarget);

    return EntityRenderContext {
        .position = attackTarget->position,
        .gameplayDistance = attackTarget->gameplayDistance,
        .color = color,
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = context.settings.objectESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = false,
        .entityType = EntityTypes::AttackTarget,
        .attitude = Game::Attitude::Neutral,
        .entity = attackTarget,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .showCombatUI = !hideCombatUI
    };
}

EntityRenderContext ContextFactory::CreateEntityRenderContextForRendering(const RenderableEntity* entity, const FrameContext& context) {
    s_DetailsBuffer.clear();
    
    switch(entity->entityType) {
        case EntityTypes::Player:
        {
            const auto* player = static_cast<const RenderablePlayer*>(entity);
            InfoBuilder::AppendPlayerDetails(player, context.settings.playerESP, context.settings.showDebugAddresses, s_DetailsBuffer);
            if (context.settings.playerESP.enableGearDisplay && context.settings.playerESP.gearDisplayMode == GearDisplayMode::Detailed) {
                if (!s_DetailsBuffer.empty()) {
                    s_DetailsBuffer.emplace_back("--- Gear Stats ---", ESPColors::DEFAULT_TEXT);
                }
                InfoBuilder::AppendGearDetails(player, s_DetailsBuffer);
            }
            break;
        }
        case EntityTypes::NPC:
        {
            const auto* npc = static_cast<const RenderableNpc*>(entity);
            InfoBuilder::AppendNpcDetails(npc, context.settings.npcESP, context.settings.showDebugAddresses, s_DetailsBuffer);
            break;
        }
        case EntityTypes::Gadget:
        {
            const auto* gadget = static_cast<const RenderableGadget*>(entity);
            InfoBuilder::AppendGadgetDetails(gadget, context.settings.objectESP, context.settings.showDebugAddresses, s_DetailsBuffer);
            break;
        }
        case EntityTypes::AttackTarget:
        {
            const auto* attackTarget = static_cast<const RenderableAttackTarget*>(entity);
            InfoBuilder::AppendAttackTargetDetails(attackTarget, context.settings.objectESP, context.settings.showDebugAddresses, s_DetailsBuffer);
            break;
        }
    }

    switch(entity->entityType) {
        case EntityTypes::Player:
            return CreateContextForPlayer(static_cast<const RenderablePlayer*>(entity), s_DetailsBuffer, context);
        case EntityTypes::NPC:
            return CreateContextForNpc(static_cast<const RenderableNpc*>(entity), s_DetailsBuffer, context);
        case EntityTypes::Gadget:
            return CreateContextForGadget(static_cast<const RenderableGadget*>(entity), s_DetailsBuffer, context);
        case EntityTypes::AttackTarget:
            return CreateContextForAttackTarget(static_cast<const RenderableAttackTarget*>(entity), s_DetailsBuffer, context);
    }
    return CreateContextForGadget(static_cast<const RenderableGadget*>(entity), s_DetailsBuffer, context);
}

} // namespace kx