#include "ESPFilter.h"

#include "AppState.h"
#include "Utils/EntityFilter.h"
#include "../Data/RenderableData.h"
#include "../Combat/CombatStateManager.h"
#include "../Utils/ESPConstants.h"

namespace kx {

namespace { // Anonymous namespace for local helpers

    bool IsDeathAnimationPlaying(const void* entityAddress, const CombatStateManager& stateManager, uint64_t now) {
        const EntityCombatState* state = stateManager.GetState(entityAddress);
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
        const DistanceSettings& distanceSettings
    ) {
        if (!entity || !entity->isValid) {
            return false;
        }

        entity->visualDistance = glm::length(entity->position - cameraPos);
        entity->gameplayDistance = glm::length(entity->position - playerPos);

        if (distanceSettings.useDistanceLimit && entity->gameplayDistance > distanceSettings.renderDistanceLimit) {
            return false;
        }

        return true;
    }

} // anonymous namespace

void ESPFilter::FilterPooledData(const PooledFrameRenderData& extractedData, Camera& camera,
                                 PooledFrameRenderData& filteredData, const CombatStateManager& stateManager, uint64_t now) {
    filteredData.Reset();
    
    const auto& settings = AppState::Get().GetSettings();
    const glm::vec3 playerPos = camera.GetPlayerPosition();
    const glm::vec3 cameraPos = camera.GetCameraPosition();
    
    // Filter players
    if (settings.playerESP.enabled) {
        filteredData.players.reserve(extractedData.players.size());
        for (RenderablePlayer* player : extractedData.players) {
            // Call the common helper function first
            if (!PassesCommonFilters(player, cameraPos, playerPos, settings.distance)) {
                continue;
            }
            
            // Now, perform player-specific filtering
            if (player->isLocalPlayer && !settings.playerESP.showLocalPlayer) continue;
            
            if (player->currentHealth <= 0.0f && !IsDeathAnimationPlaying(player->address, stateManager, now)) {
                continue;
            }
            
            if (!Filtering::EntityFilter::ShouldRenderPlayer(player->attitude, settings.playerESP)) continue;
            
            filteredData.players.push_back(player);
        }
    }
    
    // Filter NPCs
    if (settings.npcESP.enabled) {
        filteredData.npcs.reserve(extractedData.npcs.size());
        for (RenderableNpc* npc : extractedData.npcs) {
            // Call the common helper function first
            if (!PassesCommonFilters(npc, cameraPos, playerPos, settings.distance)) {
                continue;
            }
            
            // Now, perform NPC-specific filtering
            if (npc->currentHealth <= 0.0f && !settings.npcESP.showDeadNpcs && !IsDeathAnimationPlaying(npc->address, stateManager, now)) {
                continue;
            }
            
            if (!Filtering::EntityFilter::ShouldRenderNpc(npc->attitude, npc->rank, settings.npcESP)) continue;
            
            filteredData.npcs.push_back(npc);
        }
    }
    
    // Filter gadgets
    if (settings.objectESP.enabled) {
        filteredData.gadgets.reserve(extractedData.gadgets.size());
        for (RenderableGadget* gadget : extractedData.gadgets) {
            // Call the common helper function first
            if (!PassesCommonFilters(gadget, cameraPos, playerPos, settings.distance)) {
                continue;
            }

            // Now, perform gadget-specific filtering
            if (gadget->maxHealth > 0 && gadget->currentHealth <= 0.0f && !settings.objectESP.showDeadGadgets && !IsDeathAnimationPlaying(gadget->address, stateManager, now)) {
                continue;
            }

            if (settings.hideDepletedNodes && gadget->type == Game::GadgetType::ResourceNode && !gadget->isGatherable) {
                continue;
            }
            
            if (!Filtering::EntityFilter::ShouldRenderGadget(gadget->type, settings.objectESP)) continue;
            
            // Filter boxes for oversized gadgets (world bosses, huge structures)
            // This prevents screen clutter from massive 20-30m tall entities
            if (settings.objectESP.renderBox && gadget->hasPhysicsDimensions) {
                if (gadget->physicsHeight > settings.objectESP.maxBoxHeight) {
                    // Gadget is too tall - don't render it (will be filtered out)
                    // Note: We could alternatively just disable the box, but filtering
                    // the entire gadget is cleaner since giant bosses are usually obvious
                    continue;
                }
            }
            
            filteredData.gadgets.push_back(gadget);
        }
    }
    
    // Filter attack targets
    if (settings.objectESP.enabled && settings.objectESP.showAttackTargetList) {
        filteredData.attackTargets.reserve(extractedData.attackTargets.size());
        for (RenderableAttackTarget* attackTarget : extractedData.attackTargets) {
            // Call the common helper function first
            if (!PassesCommonFilters(attackTarget, cameraPos, playerPos, settings.distance)) {
                continue;
            }

            // Filter boxes for oversized attack targets (walls, large structures)
            // This prevents screen clutter from massive 20-30m tall entities
            if (settings.objectESP.renderBox && attackTarget->hasPhysicsDimensions) {
                if (attackTarget->physicsHeight > settings.objectESP.maxBoxHeight) {
                    // Attack target is too tall - don't render it (will be filtered out)
                    continue;
                }
            }
            
            filteredData.attackTargets.push_back(attackTarget);
        }
    }
}

} // namespace kx
