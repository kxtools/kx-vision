#include "StageRenderer.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Shared/LayoutConstants.h"
#include "../Renderers/EntityComponentRenderer.h"
#include "../Renderers/LayoutCursor.h"
#include "../Renderers/TrailRenderer.h"
#include "../Data/RenderableData.h"
#include "../Data/HealthBarAnimationState.h"
#include "../Presentation/Styling.h"
#include "../Shared/ColorConstants.h"
#include "../Logic/StyleCalculator.h"
#include "../Logic/Animations/HealthBarAnimations.h"
#include "../Combat/CombatStateManager.h"
#include "../Combat/CombatState.h"
#include "../Combat/CombatConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <cstdio>

#include "Renderers/ScreenProjector.h"
#include "Shared/RenderSettingsHelper.h"

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

bool ShouldRenderPlayerHealthBar(const RenderablePlayer& player, const PlayerEspSettings& settings) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (settings.showOnlyDamaged && player.maxHealth > 0 && player.currentHealth >= player.maxHealth) {
        return false;
    }
    return true;
}

bool ShouldRenderNpcHealthBar(const RenderableNpc& npc,
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

bool ShouldRenderGadgetHealthBar(const RenderableGadget& gadget,
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

std::string_view GetEntityName(const RenderableEntity& entity) {
    switch (entity.entityType) {
        case EntityTypes::Player:
            return static_cast<const RenderablePlayer&>(entity).playerName;
        case EntityTypes::NPC:
            return static_cast<const RenderableNpc&>(entity).name;
        case EntityTypes::Gadget:
            return static_cast<const RenderableGadget&>(entity).name;
        case EntityTypes::Item: {
            static thread_local char itemNameBuffer[64];
            const auto& item = static_cast<const RenderableItem&>(entity);
            sprintf_s(itemNameBuffer, "Item [%u]", item.itemId);
            return itemNameBuffer;
        }
        default:
            return {};
    }
}


void ProcessAndRender(const FrameContext& context, const RenderableEntity* entity) {
    if (!entity) {
        return;
    }

    VisualProperties visuals;
    if (!Logic::StyleCalculator::Calculate(*entity, context, visuals.style)) {
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
            const auto& player = static_cast<const RenderablePlayer&>(*entity);
            attitude = player.attitude;
            renderHealthBar = ShouldRenderPlayerHealthBar(player, context.settings.playerESP);
            renderEnergyBar = context.settings.playerESP.renderEnergyBar;
            showCombatUI = true;
            burstDps = CalculateBurstDps(combatState, context.now, context.settings.playerESP.showBurstDps);
            break;
        }
        case EntityTypes::NPC: {
            const auto& npc = static_cast<const RenderableNpc&>(*entity);
            attitude = npc.attitude;
            renderHealthBar = ShouldRenderNpcHealthBar(npc, context.settings.npcESP, combatState, context.now);
            showCombatUI = true;
            burstDps = CalculateBurstDps(combatState, context.now, context.settings.npcESP.showBurstDps);
            break;
        }
        case EntityTypes::Gadget: {
            const auto& gadget = static_cast<const RenderableGadget&>(*entity);
            renderHealthBar = ShouldRenderGadgetHealthBar(gadget, context.settings.objectESP, combatState, context.now);
            showCombatUI = !Styling::ShouldHideCombatUIForGadget(gadget.type);
            burstDps = CalculateBurstDps(combatState, context.now, context.settings.objectESP.showBurstDps);
            break;
        }
        case EntityTypes::AttackTarget: {
            renderHealthBar = false;
            showCombatUI = true;
            burstDps = CalculateBurstDps(combatState, context.now, context.settings.objectESP.showBurstDps);
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
    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(context.settings, entity->entityType);
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
    EntityComponentRenderer::RenderGeometry(context, *entity, visuals);

    // B. Identity
    std::string_view name = GetEntityName(*entity);
    EntityComponentRenderer::RenderIdentity(context, *entity, name, visuals, cursor);

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
                                              cursor);

    // D. Details
    EntityComponentRenderer::RenderEntityDetails(context, *entity, visuals, cursor);

    // E. Trails (Player specific)
    if (entity->entityType == EntityTypes::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(entity);
        if (player) {
            TrailRenderer::RenderPlayerTrail(context, *player, attitude, visuals);
        }
    }
}

} // namespace

void StageRenderer::RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData) {
    for (const auto* player : frameData.players) {
        ProcessAndRender(context, player);
    }

    for (const auto* npc : frameData.npcs) {
        ProcessAndRender(context, npc);
    }

    for (const auto* gadget : frameData.gadgets) {
        ProcessAndRender(context, gadget);
    }

    for (const auto* target : frameData.attackTargets) {
        ProcessAndRender(context, target);
    }

    for (const auto* item : frameData.items) {
        ProcessAndRender(context, item);
    }
}

} // namespace kx
