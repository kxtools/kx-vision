#include "EntityFilter.h"

#include "FilterSettings.h"
#include "../Data/RenderableData.h"
#include "../Combat/CombatStateManager.h"
#include "Camera.h"
#include "Shared/CombatConstants.h"

namespace kx {

namespace { // Anonymous namespace for local helpers

    bool IsDeathAnimationPlaying(const RenderableEntity* entity, const CombatStateManager& stateManager, uint64_t now) {
        const EntityCombatState* state = stateManager.GetState(entity->GetCombatKey());
        if (!state || state->deathTimestamp == 0) {
            return false;
        }
        return (now - state->deathTimestamp) <= CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS;
    }

    /**
     * @brief Performs common filtering logic applicable to all entity types.
     * @return True if the entity passes common filters, false otherwise.
     */
    bool PassesCommonFilters(
        RenderableEntity* entity,
        const glm::vec3& cameraPos,
        const glm::vec3& playerPos,
        const FrameContext& context
    ) {
        if (!entity || !entity->isValid) {
            return false;
        }

        entity->visualDistance = glm::length(entity->position - cameraPos);
        entity->gameplayDistance = glm::length(entity->position - playerPos);

        float activeLimit = context.settings.distance.GetActiveDistanceLimit(entity->entityType, context.isInWvW);
        if (activeLimit > 0.0f && entity->gameplayDistance > activeLimit) {
            return false;
        }

        return true;
    }

} // anonymous namespace

void EntityFilter::FilterPooledData(const PooledFrameRenderData& extractedData, const FrameContext& context,
                                 PooledFrameRenderData& filteredData) {
    filteredData.Reset();
    
    const glm::vec3 playerPos = context.camera.GetPlayerPosition();
    const glm::vec3 cameraPos = context.camera.GetCameraPosition();
    
    // Filter players
    if (context.settings.playerESP.enabled) {
        filteredData.players.reserve(extractedData.players.size());
        for (RenderablePlayer* player : extractedData.players) {
            // Call the common helper function first
            if (!PassesCommonFilters(player, cameraPos, playerPos, context)) {
                continue;
            }
            
            // Now, perform player-specific filtering
            if (player->isLocalPlayer && !context.settings.playerESP.showLocalPlayer) continue;
            
            if (player->currentHealth <= 0.0f && !IsDeathAnimationPlaying(player, context.stateManager, context.now)) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderPlayer(player->attitude, context.settings.playerESP)) continue;
            
            filteredData.players.push_back(player);
        }
    }
    
    // Filter NPCs
    if (context.settings.npcESP.enabled) {
        filteredData.npcs.reserve(extractedData.npcs.size());
        for (RenderableNpc* npc : extractedData.npcs) {
            // Call the common helper function first
            if (!PassesCommonFilters(npc, cameraPos, playerPos, context)) {
                continue;
            }
            
            // Now, perform NPC-specific filtering
            if (npc->currentHealth <= 0.0f && !context.settings.npcESP.showDeadNpcs && !IsDeathAnimationPlaying(npc, context.stateManager, context.now)) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderNpc(npc->attitude, npc->rank, context.settings.npcESP)) continue;
            
            filteredData.npcs.push_back(npc);
        }
    }
    
    // Filter gadgets
    if (context.settings.objectESP.enabled) {
        filteredData.gadgets.reserve(extractedData.gadgets.size());
        for (RenderableGadget* gadget : extractedData.gadgets) {
            // Call the common helper function first
            if (!PassesCommonFilters(gadget, cameraPos, playerPos, context)) {
                continue;
            }

            // Now, perform gadget-specific filtering
            if (gadget->maxHealth > 0 && gadget->currentHealth <= 0.0f && !context.settings.objectESP.showDeadGadgets && !IsDeathAnimationPlaying(gadget, context.stateManager, context.now)) {
                continue;
            }

            if (context.settings.hideDepletedNodes && gadget->type == Game::GadgetType::ResourceNode && !gadget->isGatherable) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderGadget(gadget->type, context.settings.objectESP)) continue;
            
            // Note: Max height check is handled in context factory to disable box rendering only
            // Entity is still rendered with other visualizations (circles, dots, details, etc.)
            
            filteredData.gadgets.push_back(gadget);
        }
    }
    
    // Filter attack targets
    if (context.settings.objectESP.enabled && context.settings.objectESP.showAttackTargetList) {
        filteredData.attackTargets.reserve(extractedData.attackTargets.size());
        for (RenderableAttackTarget* attackTarget : extractedData.attackTargets) {
            // Call the common helper function first
            if (!PassesCommonFilters(attackTarget, cameraPos, playerPos, context)) {
                continue;
            }

            // Filter by combat state if enabled
            if (context.settings.objectESP.showAttackTargetListOnlyInCombat) {
                if (attackTarget->combatState != Game::AttackTargetCombatState::InCombat) {
                    continue;
                }
            }

            // Note: Max height check is handled in context factory to disable box rendering only
            // Entity is still rendered with other visualizations (circles, dots, details, etc.)
            
            filteredData.attackTargets.push_back(attackTarget);
        }
    }
}

} // namespace kx
