#include "ContextFactory.h"

#include "../Combat/CombatStateManager.h" // For CombatStateManager
#include "../Utils/ESPConstants.h" // For CombatEffects
#include "../Data/ESPData.h"
#include "../Data/EntityRenderContext.h"
#include "../../Game/GameEnums.h"
#include "../Animations/HealthBarAnimations.h"
#include "../Data/ESPEntityTypes.h"
#include "Settings/ESPSettings.h"
#include "InfoBuilder.h"
#include "ESPStyling.h"

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

EntityRenderContext ContextFactory::CreateContextForPlayer(const RenderablePlayer* player, const std::vector<ColoredDetail>& details, const FrameContext& context) {
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
        .position = player->position,
        .gameplayDistance = player->gameplayDistance,
        .color = color,
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = !details.empty(),
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = context.settings.playerESP.renderEnergyBar,
        .entityType = ESPEntityType::Player,
        .attitude = player->attitude,
        .entity = player,
        .playerName = player->playerName,
        .healthBarAnim = animState,
        .showCombatUI = true
    };
}

EntityRenderContext ContextFactory::CreateContextForNpc(const RenderableNpc* npc, const std::vector<ColoredDetail>& details, const FrameContext& context) {
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
        .position = npc->position,
        .gameplayDistance = npc->gameplayDistance,
        .color = color,
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = context.settings.npcESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = false, // No energy bar for NPCs
        .entityType = ESPEntityType::NPC,
        .attitude = npc->attitude,
        .entity = npc,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .showCombatUI = true
    };
}

EntityRenderContext ContextFactory::CreateContextForGadget(const RenderableGadget* gadget, const std::vector<ColoredDetail>& details, const FrameContext& context) {
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

    // Check if combat UI should be hidden for this gadget type
    bool hideCombatUI = ESPStyling::ShouldHideCombatUIForGadget(gadget->type);

    return EntityRenderContext{
        .position = gadget->position,
        .gameplayDistance = gadget->gameplayDistance,
        .color = ESPStyling::GetEntityColor(*gadget),
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = context.settings.objectESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = false, // No energy bar for gadgets
        .entityType = ESPEntityType::Gadget,
        .attitude = Game::Attitude::Neutral,
        .entity = gadget,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .showCombatUI = !hideCombatUI
    };
}

EntityRenderContext ContextFactory::CreateContextForAttackTarget(const RenderableAttackTarget* attackTarget, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    static const std::string emptyPlayerName = "";

    const EntityCombatState* state = context.stateManager.GetState(attackTarget->address);
    bool renderHealthBar = false; // Attack targets typically don't have health data

    // --- Animation State --- 
    HealthBarAnimationState animState;
    // Attack targets don't typically have health, so no animation state needed

    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.objectESP.showBurstDps);

    bool hideCombatUI = false; // Can be customized if needed

    unsigned int color = ESPStyling::GetEntityColor(*attackTarget);

    return EntityRenderContext {
        .position = attackTarget->position,
        .gameplayDistance = attackTarget->gameplayDistance,
        .color = color,
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderDetails = context.settings.objectESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderEnergyBar = false,
        .entityType = ESPEntityType::AttackTarget,
        .attitude = Game::Attitude::Neutral,
        .entity = attackTarget,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .showCombatUI = !hideCombatUI
    };
}

EntityRenderContext ContextFactory::CreateEntityRenderContextForRendering(const RenderableEntity* entity, const FrameContext& context) {
    std::vector<ColoredDetail> details;
    // Use a switch on entity->entityType to call the correct details builder
    switch(entity->entityType) {
        case ESPEntityType::Player:
        {
            const auto* player = static_cast<const RenderablePlayer*>(entity);
            details = InfoBuilder::BuildPlayerDetails(player, context.settings.playerESP, context.settings.showDebugAddresses);
            if (context.settings.playerESP.enableGearDisplay && context.settings.playerESP.gearDisplayMode == GearDisplayMode::Detailed) {
                auto gearDetails = InfoBuilder::BuildGearDetails(player);
                if (!gearDetails.empty()) {
                    if (!details.empty()) {
                        details.push_back({ "--- Gear Stats ---", ESPColors::DEFAULT_TEXT });
                    }
                    details.insert(details.end(), gearDetails.begin(), gearDetails.end());
                }
            }
            break;
        }
        case ESPEntityType::NPC:
        {
            const auto* npc = static_cast<const RenderableNpc*>(entity);
            details = InfoBuilder::BuildNpcDetails(npc, context.settings.npcESP, context.settings.showDebugAddresses);
            break;
        }
        case ESPEntityType::Gadget:
        {
            const auto* gadget = static_cast<const RenderableGadget*>(entity);
            details = InfoBuilder::BuildGadgetDetails(gadget, context.settings.objectESP, context.settings.showDebugAddresses);
            break;
        }
        case ESPEntityType::AttackTarget:
        {
            const auto* attackTarget = static_cast<const RenderableAttackTarget*>(entity);
            details = InfoBuilder::BuildAttackTargetDetails(attackTarget, context.settings.objectESP, context.settings.showDebugAddresses);
            break;
        }
    }

    // Now, create the context using the ESPContextFactory, just like before.
    // We pass the main 'context' directly.
    switch(entity->entityType) {
        case ESPEntityType::Player:
            return CreateContextForPlayer(static_cast<const RenderablePlayer*>(entity), details, context);
        case ESPEntityType::NPC:
            return CreateContextForNpc(static_cast<const RenderableNpc*>(entity), details, context);
        case ESPEntityType::Gadget:
            return CreateContextForGadget(static_cast<const RenderableGadget*>(entity), details, context);
        case ESPEntityType::AttackTarget:
            return CreateContextForAttackTarget(static_cast<const RenderableAttackTarget*>(entity), details, context);
    }
    // This should not be reached, but we need to return something.
    // Returning a gadget context as a fallback.
    return CreateContextForGadget(static_cast<const RenderableGadget*>(entity), details, context);
}

} // namespace kx