#include "ESPFilter.h"

#include "AppState.h"
#include "Utils/EntityFilter.h"
#include "../Data/RenderableData.h"
#include "../Combat/CombatStateManager.h"
#include "../Utils/ESPConstants.h"
#include <Windows.h>

namespace kx {

namespace { // Anonymous namespace for local helpers

    /**
     * @brief Checks if an entity is considered "dead" but should still be rendered for its death animation.
     * @param entityAddress The memory address of the entity.
     * @param stateManager The combat state manager.
     * @return True if the death animation is still considered to be playing, false otherwise.
     */
    bool IsDeathAnimationPlaying(const void* entityAddress, const CombatStateManager& stateManager) {
        const EntityCombatState* state = stateManager.GetState(entityAddress);
        if (!state || state->deathTimestamp == 0) {
            return false; // Entity is not marked as dead in the combat state.
        }
        // Check if the time since death is within the total animation duration.
        return (GetTickCount64() - state->deathTimestamp) <= CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS;
    }

} // anonymous namespace

void ESPFilter::FilterPooledData(const PooledFrameRenderData& extractedData, Camera& camera,
                                 PooledFrameRenderData& filteredData, const CombatStateManager& stateManager) {
    filteredData.Reset();
    
    const auto& settings = AppState::Get().GetSettings();
    const glm::vec3 playerPos = camera.GetPlayerPosition();
    const glm::vec3 cameraPos = camera.GetCameraPosition();
    
    // Filter players - apply same logic but work with pointers
    if (settings.playerESP.enabled) {
        filteredData.players.reserve(extractedData.players.size());
        for (RenderablePlayer* player : extractedData.players) {
            if (!player || !player->isValid) continue;
            
            // Apply local player filter
            if (player->isLocalPlayer && !settings.playerESP.showLocalPlayer) continue;
            
            // Apply health filter (refactored)
            if (player->currentHealth <= 0.0f) {
                if (!IsDeathAnimationPlaying(player->address, stateManager)) {
                    continue; // Cull dead player if animation is finished.
                }
            }
            
            // Calculate distances
            player->visualDistance = glm::length(player->position - cameraPos);
            player->gameplayDistance = glm::length(player->position - playerPos);
            
            // Apply distance filter based on visual distance
            if (settings.distance.useDistanceLimit && player->gameplayDistance > settings.distance.renderDistanceLimit) continue;
            
            // Apply attitude-based filter
            if (!Filtering::EntityFilter::ShouldRenderPlayer(player->attitude, settings.playerESP)) continue;
            
            filteredData.players.push_back(player);
        }
    }
    
    // Filter NPCs - apply same logic but work with pointers
    if (settings.npcESP.enabled) {
        filteredData.npcs.reserve(extractedData.npcs.size());
        for (RenderableNpc* npc : extractedData.npcs) {
            if (!npc || !npc->isValid) continue;
            
            // Apply health filter (refactored)
            if (npc->currentHealth <= 0.0f && !settings.npcESP.showDeadNpcs) {
                if (!IsDeathAnimationPlaying(npc->address, stateManager)) {
                    continue; // Cull dead NPC if setting is off and animation is finished.
                }
            }
            
            // Calculate distances
            npc->visualDistance = glm::length(npc->position - cameraPos);
            npc->gameplayDistance = glm::length(npc->position - playerPos);
            
            // Apply distance filter based on visual distance
            if (settings.distance.useDistanceLimit && npc->gameplayDistance > settings.distance.renderDistanceLimit) continue;
            
            // Apply attitude- and rank-based filter
            if (!Filtering::EntityFilter::ShouldRenderNpc(npc->attitude, npc->rank, settings.npcESP)) continue;
            
            filteredData.npcs.push_back(npc);
        }
    }
    
    // Filter gadgets - apply same logic but work with pointers
    if (settings.objectESP.enabled) {
        filteredData.gadgets.reserve(extractedData.gadgets.size());
        for (RenderableGadget* gadget : extractedData.gadgets) {
            if (!gadget || !gadget->isValid) continue;

            // Handle dead gadgets.
            if (gadget->maxHealth > 0 && gadget->currentHealth <= 0.0f) {
                if (!settings.objectESP.showDeadGadgets) {
                    if (!IsDeathAnimationPlaying(gadget->address, stateManager)) {
                        continue; // Cull if setting is off and animation is done.
                    }
                }
            }

            // Calculate distances
            gadget->visualDistance = glm::length(gadget->position - cameraPos);
            gadget->gameplayDistance = glm::length(gadget->position - playerPos);
            
            // Apply distance filter based on visual distance
            if (settings.distance.useDistanceLimit && gadget->gameplayDistance > settings.distance.renderDistanceLimit) continue;
            
            // Apply gadget type-based filter
            if (!Filtering::EntityFilter::ShouldRenderGadget(gadget->type, settings.objectESP)) continue;
            
            // Apply depleted resource node filter
            if (settings.hideDepletedNodes && gadget->type == Game::GadgetType::ResourceNode && !gadget->isGatherable) {
                continue;
            }
            
            filteredData.gadgets.push_back(gadget);
        }
    }
}

} // namespace kx