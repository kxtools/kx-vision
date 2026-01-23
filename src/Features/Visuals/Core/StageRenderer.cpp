#include "StageRenderer.h"
#include "../../../Game/Services/Camera/Camera.h"
#include "../Renderers/EntityComponentRenderer.h"
#include "../Renderers/LayoutCursor.h"
#include "../Renderers/TrailRenderer.h"
#include "../../../Game/Data/EntityData.h"
#include "../../../Rendering/Data/HealthBarAnimationState.h"
#include "../Presentation/Styling.h"
#include "../Logic/StyleCalculator.h"
#include "../Logic/Animations/HealthBarAnimations.h"
#include "../../../Game/Services/Combat/CombatStateManager.h"
#include "../../../Game/Services/Combat/CombatState.h"
#include "../../../Game/Services/Combat/CombatConstants.h"
#include "../Settings/VisualsSettings.h"
#include "../../../../libs/ImGui/imgui.h"
#include <string_view>
#include <vector>

#include "../Renderers/ScreenProjector.h"
#include "../../../Rendering/Shared/RenderSettingsHelper.h"

namespace kx {

namespace {

float CalculateBurstDps(const EntityCombatState* state, uint64_t now, bool showBurstDpsSetting) {
    if (!showBurstDpsSetting || !state || state->burstStartTime == 0 || state->accumulatedDamage <= 0.0f) {
        return 0.0f;
    }

    uint64_t durationMs = now - state->burstStartTime;
    if (durationMs <= 100) {
        return 0.0f;
    }

    float durationSeconds = static_cast<float>(durationMs) / 1000.0f;
    return durationSeconds > 0.0f ? (state->accumulatedDamage / durationSeconds) : 0.0f;
}

bool IsDeathAnimating(const EntityCombatState* state, uint64_t now) {
    if (!state || state->deathTimestamp == 0) {
        return false;
    }
    return (now - state->deathTimestamp) <= CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS;
}

bool ShouldRenderPlayerHealthBar(const PlayerEntity& player, const PlayerEspSettings& settings) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (settings.showOnlyDamaged && player.maxHealth > 0 && player.currentHealth >= player.maxHealth) {
        return false;
    }
    return true;
}

bool ShouldRenderNpcHealthBar(const NpcEntity& npc,
                              const NpcEspSettings& settings,
                              const EntityCombatState* state,
                              uint64_t now) {
    if (!settings.renderHealthBar) {
        return false;
    }

    const bool deathAnimating = IsDeathAnimating(state, now);

    if (settings.showOnlyDamaged &&
        npc.maxHealth > 0 &&
        npc.currentHealth >= npc.maxHealth &&
        !deathAnimating) {
        return false;
    }

    if (!settings.showDeadNpcs &&
        npc.currentHealth <= 0.0f &&
        !deathAnimating) {
        return false;
    }
    return true;
}

bool ShouldRenderGadgetHealthBar(const GadgetEntity& gadget,
                                 const ObjectEspSettings& settings,
                                 const EntityCombatState* state,
                                 uint64_t now) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (Styling::ShouldHideCombatUIForGadget(gadget.type)) {
        return false;
    }
    const bool deathAnimating = IsDeathAnimating(state, now);

    if (gadget.maxHealth <= 0.0f && !deathAnimating) {
        return false;
    }
    if (settings.showOnlyDamaged &&
        gadget.currentHealth >= gadget.maxHealth &&
        !deathAnimating) {
        return false;
    }
    if (gadget.currentHealth <= 0.0f && !deathAnimating) {
        if (!settings.showDeadGadgets) {
            return false;
        }
    }
    return true;
}

std::string_view GetEntityName(const GameEntity& entity) {
    switch (entity.entityType) {
        case EntityTypes::Player:
            return static_cast<const PlayerEntity&>(entity).playerName;
        case EntityTypes::NPC:
            return static_cast<const NpcEntity&>(entity).name;
        case EntityTypes::Gadget:
            return static_cast<const GadgetEntity&>(entity).name;
        case EntityTypes::Item: {
            static thread_local char itemNameBuffer[64];
            const auto& item = static_cast<const ItemEntity&>(entity);
            auto res = std::format_to_n(itemNameBuffer, std::size(itemNameBuffer), "Item [{}]", item.itemId);
            return std::string_view(itemNameBuffer, res.size);
        }
        default:
            return {};
    }
}


void ProcessAndRender(const FrameContext& context, const GameEntity* entity, const VisualsConfiguration& visualsConfig) {
    if (!entity) {
        return;
    }

    VisualProperties visuals;
    if (!Logic::StyleCalculator::Calculate(*entity, context, visualsConfig, visuals.style)) {
        return;
    }

    bool isOnScreen = Renderers::ScreenProjector::Project(
        *entity,
        context.camera,
        context.screenWidth,
        context.screenHeight,
        visuals.style,
        visuals.geometry);

    if (!isOnScreen) {
        return;
    }

    const EntityCombatState* combatState = context.stateManager.GetState(entity->GetCombatKey());
    
    bool showCombatUI = true;
    bool renderHealthBar = false;
    bool renderEnergyBar = false;
    float burstDps = 0.0f;
    Game::Attitude attitude = Game::Attitude::Neutral;

    switch (entity->entityType) {
        case EntityTypes::Player: {
            const auto& player = static_cast<const PlayerEntity&>(*entity);
            attitude = player.attitude;
            renderHealthBar = ShouldRenderPlayerHealthBar(player, visualsConfig.playerESP);
            renderEnergyBar = visualsConfig.playerESP.renderEnergyBar;
            showCombatUI = true;
            burstDps = CalculateBurstDps(combatState, context.now, visualsConfig.playerESP.showBurstDps);
            break;
        }
        case EntityTypes::NPC: {
            const auto& npc = static_cast<const NpcEntity&>(*entity);
            attitude = npc.attitude;
            renderHealthBar = ShouldRenderNpcHealthBar(npc, visualsConfig.npcESP, combatState, context.now);
            showCombatUI = true;
            burstDps = CalculateBurstDps(combatState, context.now, visualsConfig.npcESP.showBurstDps);
            break;
        }
        case EntityTypes::Gadget: {
            const auto& gadget = static_cast<const GadgetEntity&>(*entity);
            renderHealthBar = ShouldRenderGadgetHealthBar(gadget, visualsConfig.objectESP, combatState, context.now);
            showCombatUI = !Styling::ShouldHideCombatUIForGadget(gadget.type);
            burstDps = CalculateBurstDps(combatState, context.now, visualsConfig.objectESP.showBurstDps);
            break;
        }
        case EntityTypes::AttackTarget: {
            renderHealthBar = false;
            showCombatUI = true;
            burstDps = CalculateBurstDps(combatState, context.now, visualsConfig.objectESP.showBurstDps);
            break;
        }
        case EntityTypes::Item: {
            renderHealthBar = false;
            showCombatUI = false;
            break;
        }
    }

    HealthBarAnimationState animState;
    if (renderHealthBar && combatState) {
        PopulateHealthBarAnimations(entity, combatState, animState, context.now);
    }

    // RENDER PHASE (Immediate Mode)
    // Initialize cursor at top of entity
    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(visualsConfig, entity->entityType);
    LayoutCursor cursor({visuals.geometry.center.x, visuals.geometry.boxMax.y}, 1.0f);

    // Special case for gadgets without boxes
    if (entity->entityType == EntityTypes::Gadget && !shouldRenderBox) {
        cursor = LayoutCursor(glm::vec2(visuals.geometry.screenPos.x, visuals.geometry.screenPos.y), 1.0f);
    }
    
    // Special case for items without boxes
    if (entity->entityType == EntityTypes::Item && !shouldRenderBox) {
        cursor = LayoutCursor(glm::vec2(visuals.geometry.screenPos.x, visuals.geometry.screenPos.y), 1.0f);
    }

    // A. Geometry
    EntityComponentRenderer::RenderGeometry(context, *entity, visuals, visualsConfig);

    // B. Identity
    std::string_view name = GetEntityName(*entity);
    EntityComponentRenderer::RenderIdentity(context, *entity, name, visuals, cursor, visualsConfig);

    // C. Bars
    EntityComponentRenderer::RenderStatusBars(context,
                                              *entity,
                                              showCombatUI,
                                              renderHealthBar,
                                              renderEnergyBar,
                                              burstDps,
                                              attitude,
                                              animState,
                                              visuals,
                                              cursor,
                                              visualsConfig);

    // D. Details
    EntityComponentRenderer::RenderEntityDetails(context, *entity, visuals, cursor, visualsConfig);

    // E. Trails (Player specific)
    if (entity->entityType == EntityTypes::Player) {
        const auto* player = static_cast<const PlayerEntity*>(entity);
        if (player) {
            TrailRenderer::RenderPlayerTrail(context, *player, attitude, visuals, visualsConfig);
        }
    }
}

} // namespace

void StageRenderer::RenderFrameData(const FrameContext& context, const FrameGameData& frameData, const VisualsConfiguration& visualsConfig) {
    for (const auto* player : frameData.players) {
        ProcessAndRender(context, player, visualsConfig);
    }

    for (const auto* npc : frameData.npcs) {
        ProcessAndRender(context, npc, visualsConfig);
    }

    for (const auto* gadget : frameData.gadgets) {
        ProcessAndRender(context, gadget, visualsConfig);
    }

    for (const auto* target : frameData.attackTargets) {
        ProcessAndRender(context, target, visualsConfig);
    }

    for (const auto* item : frameData.items) {
        ProcessAndRender(context, item, visualsConfig);
    }
}

} // namespace kx
