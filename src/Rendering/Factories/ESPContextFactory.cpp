#include "ESPContextFactory.h"

#include "../Combat/CombatStateManager.h" // For CombatStateManager
#include "../Utils/ESPConstants.h" // For CombatEffects
#include "../Data/ESPData.h"
#include "../Data/EntityRenderContext.h"
#include "../../Game/GameEnums.h"
#include "../Utils/ESPStyling.h"
#include "../Animations/HealthBarAnimations.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPEntityDetailsBuilder.h"
#include "../Data/ESPEntityTypes.h"

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
        .renderBox = context.settings.playerESP.renderBox,
        .renderDistance = context.settings.playerESP.renderDistance,
        .renderDot = context.settings.playerESP.renderDot,
        .renderDetails = !details.empty(),
        .renderHealthBar = renderHealthBar,
        .renderHealthPercentage = context.settings.playerESP.showHealthPercentage,
        .renderEnergyBar = context.settings.playerESP.renderEnergyBar,
        .renderPlayerName = context.settings.playerESP.renderPlayerName,
        .entityType = ESPEntityType::Player,
        .attitude = player->attitude,
        .entity = player,
        .playerName = player->playerName,
        .healthBarAnim = animState,
        .renderGadgetSphere = false,
        .renderGadgetCircle = false,
        .playerGearDisplayMode = context.settings.playerESP.gearDisplayMode,
        .playerEnergyDisplayType = context.settings.playerESP.energyDisplayType,
        .showCombatUI = true,
        .showDamageNumbers = context.settings.playerESP.showDamageNumbers,
        .showBurstDps = context.settings.playerESP.showBurstDps
    };
}

EntityRenderContext ESPContextFactory::CreateContextForNpc(const RenderableNpc* npc, const std::vector<ColoredDetail>& details, const FrameContext& context) {
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
        .renderBox = context.settings.npcESP.renderBox,
        .renderDistance = context.settings.npcESP.renderDistance,
        .renderDot = context.settings.npcESP.renderDot,
        .renderDetails = context.settings.npcESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderHealthPercentage = context.settings.npcESP.showHealthPercentage,
        .renderEnergyBar = false, // No energy bar for NPCs
        .renderPlayerName = false,
        .entityType = ESPEntityType::NPC,
        .attitude = npc->attitude,
        .entity = npc,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .renderGadgetSphere = false,
        .renderGadgetCircle = false,
        .playerGearDisplayMode = GearDisplayMode::Off,
        .playerEnergyDisplayType = EnergyDisplayType::Special,
        .showCombatUI = true,
        .showDamageNumbers = context.settings.npcESP.showDamageNumbers,
        .showBurstDps = context.settings.npcESP.showBurstDps
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

    // Check if combat UI should be hidden for this gadget type
    bool hideCombatUI = ESPStyling::ShouldHideCombatUIForGadget(gadget->type);

    // Disable box rendering for oversized gadgets (world bosses, huge structures)
    // This prevents screen clutter from massive 20-30m tall entities while still allowing
    // other visualizations (circles, dots, details, etc.) to render
    bool renderBox = context.settings.objectESP.renderBox;
    if (renderBox && gadget->hasPhysicsDimensions && gadget->physicsHeight > context.settings.objectESP.maxBoxHeight) {
        renderBox = false;
    }

    bool renderWireframe = context.settings.objectESP.renderWireframe;
    if (renderWireframe && gadget->hasPhysicsDimensions && gadget->physicsHeight > context.settings.objectESP.maxBoxHeight) {
        renderWireframe = false;
    }

    return EntityRenderContext{
        .position = gadget->position,
        .gameplayDistance = gadget->gameplayDistance,
        .color = ESPStyling::GetEntityColor(*gadget),
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderBox = renderBox,
        .renderWireframe = renderWireframe,
        .renderDistance = context.settings.objectESP.renderDistance,
        .renderDot = context.settings.objectESP.renderDot,
        .renderDetails = context.settings.objectESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderHealthPercentage = context.settings.objectESP.showHealthPercentage,
        .renderEnergyBar = false, // No energy bar for gadgets
        .renderPlayerName = false,
        .entityType = ESPEntityType::Gadget,
        .attitude = Game::Attitude::Neutral,
        .entity = gadget,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .renderGadgetSphere = context.settings.objectESP.renderSphere,
        .renderGadgetCircle = context.settings.objectESP.renderCircle,
        .playerGearDisplayMode = GearDisplayMode::Off,
        .playerEnergyDisplayType = EnergyDisplayType::Special,
        .showCombatUI = !hideCombatUI,
        .showDamageNumbers = context.settings.objectESP.showDamageNumbers,
        .showBurstDps = context.settings.objectESP.showBurstDps
    };
}

EntityRenderContext ESPContextFactory::CreateContextForAttackTarget(const RenderableAttackTarget* attackTarget, const std::vector<ColoredDetail>& details, const FrameContext& context) {
    static const std::string emptyPlayerName = "";

    const EntityCombatState* state = context.stateManager.GetState(attackTarget->address);
    bool renderHealthBar = false; // Attack targets typically don't have health data

    // --- Animation State --- 
    HealthBarAnimationState animState;
    // Attack targets don't typically have health, so no animation state needed

    float burstDpsValue = CalculateBurstDps(state, context.now, context.settings.objectESP.showBurstDps);

    bool hideCombatUI = false; // Can be customized if needed

    unsigned int color = ESPStyling::GetEntityColor(*attackTarget);

    // Disable box rendering for oversized attack targets (walls, large structures)
    // This prevents screen clutter from massive 20-30m tall entities while still allowing
    // other visualizations (circles, dots, details, etc.) to render
    bool renderBox = context.settings.objectESP.renderBox;
    if (renderBox && attackTarget->hasPhysicsDimensions && attackTarget->physicsHeight > context.settings.objectESP.maxBoxHeight) {
        renderBox = false;
    }

    bool renderWireframe = context.settings.objectESP.renderWireframe;
    if (renderWireframe && attackTarget->hasPhysicsDimensions && attackTarget->physicsHeight > context.settings.objectESP.maxBoxHeight) {
        renderWireframe = false;
    }

    return EntityRenderContext {
        .position = attackTarget->position,
        .gameplayDistance = attackTarget->gameplayDistance,
        .color = color,
        .details = std::move(details),
        .burstDPS = burstDpsValue,
        .renderBox = renderBox,
        .renderWireframe = renderWireframe,
        .renderDistance = context.settings.objectESP.renderDistance,
        .renderDot = context.settings.objectESP.renderDot,
        .renderDetails = context.settings.objectESP.renderDetails,
        .renderHealthBar = renderHealthBar,
        .renderHealthPercentage = context.settings.objectESP.showHealthPercentage,
        .renderEnergyBar = false,
        .renderPlayerName = false,
        .entityType = ESPEntityType::AttackTarget,
        .attitude = Game::Attitude::Neutral,
        .entity = attackTarget,
        .playerName = emptyPlayerName,
        .healthBarAnim = animState,
        .renderGadgetSphere = context.settings.objectESP.renderSphere,
        .renderGadgetCircle = context.settings.objectESP.renderCircle,
        .playerGearDisplayMode = GearDisplayMode::Off,
        .playerEnergyDisplayType = EnergyDisplayType::Special,
        .showCombatUI = !hideCombatUI,
        .showDamageNumbers = context.settings.objectESP.showDamageNumbers,
        .showBurstDps = context.settings.objectESP.showBurstDps
    };
}

EntityRenderContext ESPContextFactory::CreateEntityRenderContextForRendering(const RenderableEntity* entity, const FrameContext& context) {
    std::vector<ColoredDetail> details;
    // Use a switch on entity->entityType to call the correct details builder
    switch(entity->entityType) {
        case ESPEntityType::Player:
        {
            const auto* player = static_cast<const RenderablePlayer*>(entity);
            details = ESPPlayerDetailsBuilder::BuildPlayerDetails(player, context.settings.playerESP, context.settings.showDebugAddresses);
            if (context.settings.playerESP.gearDisplayMode == GearDisplayMode::Detailed) {
                auto gearDetails = ESPPlayerDetailsBuilder::BuildGearDetails(player);
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
            details = ESPEntityDetailsBuilder::BuildNpcDetails(npc, context.settings.npcESP, context.settings.showDebugAddresses);
            break;
        }
        case ESPEntityType::Gadget:
        {
            const auto* gadget = static_cast<const RenderableGadget*>(entity);
            details = ESPEntityDetailsBuilder::BuildGadgetDetails(gadget, context.settings.objectESP, context.settings.showDebugAddresses);
            break;
        }
        case ESPEntityType::AttackTarget:
        {
            const auto* attackTarget = static_cast<const RenderableAttackTarget*>(entity);
            details = ESPEntityDetailsBuilder::BuildAttackTargetDetails(attackTarget, context.settings.objectESP, context.settings.showDebugAddresses);
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