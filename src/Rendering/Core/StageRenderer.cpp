#include "StageRenderer.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Shared/LayoutConstants.h"
#include "../Renderers/EntityComponentRenderer.h"
#include "../Renderers/LayoutCursor.h"
#include "../Renderers/TrailRenderer.h"
#include "../Data/RenderableData.h"
#include "../Data/HealthBarAnimationState.h"
#include "../Presentation/InfoBuilder.h"
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
#include <type_traits>
#include <vector>

#include "Renderers/ScreenProjector.h"
#include "Shared/RenderSettingsHelper.h"

namespace kx {

namespace {

struct EntityRenderState {
    std::string displayName;
    std::vector<ColoredDetail> details;
    bool showCombatUI = true;
    bool renderHealthBar = false;
    bool renderEnergyBar = false;
    bool renderDetails = false;
    float burstDps = 0.0f;
    Game::Attitude attitude = Game::Attitude::Neutral;
};

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

bool ShouldRenderPlayerHealthBar(const RenderablePlayer& player, const PlayerEspSettings& settings) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (settings.showOnlyDamaged && player.maxHealth > 0 && player.currentHealth >= player.maxHealth) {
        return false;
    }
    return true;
}

bool ShouldRenderNpcHealthBar(const RenderableNpc& npc, const NpcEspSettings& settings) {
    if (!settings.renderHealthBar) {
        return false;
    }
    if (settings.showOnlyDamaged && npc.maxHealth > 0 && npc.currentHealth >= npc.maxHealth) {
        return false;
    }
    if (!settings.showDeadNpcs && npc.currentHealth <= 0.0f) {
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
    if (gadget.maxHealth <= 0.0f) {
        return false;
    }
    if (settings.showOnlyDamaged && gadget.currentHealth >= gadget.maxHealth) {
        return false;
    }
    if (gadget.currentHealth <= 0.0f) {
        if (!settings.showDeadGadgets) {
            return false;
        }
        if (!state || state->deathTimestamp == 0) {
            return false;
        }
        uint64_t sinceDeath = now - state->deathTimestamp;
        if (sinceDeath > CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS) {
            return false;
        }
    }
    return true;
}

std::string BuildDisplayName(const RenderableEntity& entity) {
    switch (entity.entityType) {
        case EntityTypes::Player:
            return static_cast<const RenderablePlayer&>(entity).playerName;
        case EntityTypes::NPC:
            return static_cast<const RenderableNpc&>(entity).name;
        case EntityTypes::Gadget:
            return static_cast<const RenderableGadget&>(entity).name;
        default:
            return {};
    }
}

void BuildDetailsForEntity(const RenderableEntity& entity,
                           const FrameContext& context,
                           std::vector<ColoredDetail>& out) {
    switch (entity.entityType) {
        case EntityTypes::Player: {
            const auto& player = static_cast<const RenderablePlayer&>(entity);
            InfoBuilder::AppendPlayerDetails(&player, context.settings.playerESP, context.settings.showDebugAddresses, out);
            if (context.settings.playerESP.enableGearDisplay &&
                context.settings.playerESP.gearDisplayMode == GearDisplayMode::Detailed) {
                if (!out.empty()) {
                    out.emplace_back("--- Gear Stats ---", ESPColors::DEFAULT_TEXT);
                }
                InfoBuilder::AppendGearDetails(&player, out);
            }
            break;
        }
        case EntityTypes::NPC: {
            const auto& npc = static_cast<const RenderableNpc&>(entity);
            InfoBuilder::AppendNpcDetails(&npc, context.settings.npcESP, context.settings.showDebugAddresses, out);
            break;
        }
        case EntityTypes::Gadget: {
            const auto& gadget = static_cast<const RenderableGadget&>(entity);
            InfoBuilder::AppendGadgetDetails(&gadget, context.settings.objectESP, context.settings.showDebugAddresses, out);
            break;
        }
        case EntityTypes::AttackTarget: {
            const auto& attackTarget = static_cast<const RenderableAttackTarget&>(entity);
            InfoBuilder::AppendAttackTargetDetails(&attackTarget, context.settings.objectESP, context.settings.showDebugAddresses, out);
            break;
        }
    }
}

EntityRenderState BuildRenderStateForEntity(const RenderableEntity& entity,
                                            const FrameContext& context,
                                            const EntityCombatState* combatState) {
    EntityRenderState state;
    state.displayName = BuildDisplayName(entity);
    state.attitude = Game::Attitude::Neutral;

    switch (entity.entityType) {
        case EntityTypes::Player: {
            const auto& player = static_cast<const RenderablePlayer&>(entity);
            state.attitude = player.attitude;
            state.renderHealthBar = ShouldRenderPlayerHealthBar(player, context.settings.playerESP);
            state.renderEnergyBar = context.settings.playerESP.renderEnergyBar;
            state.renderDetails = context.settings.playerESP.renderDetails;
            state.showCombatUI = true;
            state.burstDps = CalculateBurstDps(combatState, context.now, context.settings.playerESP.showBurstDps);
            if (state.renderDetails) {
                BuildDetailsForEntity(entity, context, state.details);
                state.renderDetails = !state.details.empty();
            }
            break;
        }
        case EntityTypes::NPC: {
            const auto& npc = static_cast<const RenderableNpc&>(entity);
            state.attitude = npc.attitude;
            state.renderHealthBar = ShouldRenderNpcHealthBar(npc, context.settings.npcESP);
            state.renderDetails = context.settings.npcESP.renderDetails;
            state.showCombatUI = true;
            state.burstDps = CalculateBurstDps(combatState, context.now, context.settings.npcESP.showBurstDps);
            if (state.renderDetails) {
                BuildDetailsForEntity(entity, context, state.details);
                state.renderDetails = !state.details.empty();
            }
            break;
        }
        case EntityTypes::Gadget: {
            const auto& gadget = static_cast<const RenderableGadget&>(entity);
            state.renderHealthBar = ShouldRenderGadgetHealthBar(gadget, context.settings.objectESP, combatState, context.now);
            state.renderDetails = context.settings.objectESP.renderDetails;
            state.showCombatUI = !Styling::ShouldHideCombatUIForGadget(gadget.type);
            state.burstDps = CalculateBurstDps(combatState, context.now, context.settings.objectESP.showBurstDps);
            if (state.renderDetails) {
                BuildDetailsForEntity(entity, context, state.details);
                state.renderDetails = !state.details.empty();
            }
            break;
        }
        case EntityTypes::AttackTarget: {
            state.renderHealthBar = false;
            state.renderDetails = context.settings.objectESP.renderDetails;
            state.showCombatUI = true;
            state.burstDps = CalculateBurstDps(combatState, context.now, context.settings.objectESP.showBurstDps);
            if (state.renderDetails) {
                BuildDetailsForEntity(entity, context, state.details);
                state.renderDetails = !state.details.empty();
            }
            break;
        }
    }

    return state;
}

void RenderSingleEntity(const FrameContext& context,
                        const RenderableEntity& entity,
                        const EntityRenderState& renderState,
                        const HealthBarAnimationState& animState,
                        const VisualProperties& visuals) {
    EntityComponentRenderer::RenderGeometry(context, entity, visuals);

    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(context.settings, entity.entityType);
    LayoutCursor bottomStack({visuals.geometry.center.x, visuals.geometry.boxMax.y}, 1.0f);

    if (entity.entityType == EntityTypes::Gadget && !shouldRenderBox) {
        bottomStack = LayoutCursor(glm::vec2(visuals.geometry.screenPos.x, visuals.geometry.screenPos.y), 1.0f);
    }

    EntityComponentRenderer::RenderIdentity(context, entity, renderState.displayName, visuals, bottomStack);
    EntityComponentRenderer::RenderStatusBars(context,
                                              entity,
                                              renderState.showCombatUI,
                                              renderState.renderHealthBar,
                                              renderState.renderEnergyBar,
                                              renderState.burstDps,
                                              renderState.attitude,
                                              animState,
                                              visuals,
                                              bottomStack);
    EntityComponentRenderer::RenderDetails(context,
                                           entity,
                                           renderState.renderDetails,
                                           renderState.details,
                                           visuals,
                                           bottomStack);
}

template <typename TCollection>
void RenderCollection(const FrameContext& context, const TCollection& collection) {
    for (const auto* entity : collection) {
        if (!entity) {
            continue;
        }

        auto styleOpt = Logic::StyleCalculator::Calculate(*entity, context);
        if (!styleOpt) {
            continue;
        }

        VisualProperties visuals;
        visuals.style = *styleOpt;

        bool isOnScreen = Renderers::ScreenProjector::Project(
            *entity,
            context.camera,
            context.screenWidth,
            context.screenHeight,
            visuals.style,
            visuals.geometry);

        if (!isOnScreen) {
            continue;
        }

        const EntityCombatState* combatState = context.stateManager.GetState(entity->GetCombatKey());
        EntityRenderState renderState = BuildRenderStateForEntity(*entity, context, combatState);

        HealthBarAnimationState animState;
        if (renderState.renderHealthBar && combatState) {
            PopulateHealthBarAnimations(entity, combatState, animState, context.now);
        }

        RenderSingleEntity(context, *entity, renderState, animState, visuals);

        if constexpr (std::is_same_v<typename TCollection::value_type, RenderablePlayer*>) {
            TrailRenderer::RenderPlayerTrail(context, *entity, renderState.attitude, visuals);
        }
    }
}

} // namespace

void StageRenderer::RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData) {
    RenderCollection(context, frameData.players);
    RenderCollection(context, frameData.npcs);
    RenderCollection(context, frameData.gadgets);
    RenderCollection(context, frameData.attackTargets);
}

} // namespace kx
