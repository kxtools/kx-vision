#include "ESPFilter.h"

#include "AppState.h"
#include "EntityFilter.h"
#include "../Data/RenderableData.h"
#include "../Combat/CombatStateManager.h"
#include "../Utils/ESPConstants.h"
#include <Windows.h>

namespace kx {

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
            
            // Apply health filter
            bool isAlive = player->currentHealth > 0.0f;
            if (!isAlive) { // Player is defeated (or downed and we can't tell, but HP is 0)
                // The user wants to hide dead players, so we should filter this one out...
                // UNLESS the death animation is still playing.
                const EntityCombatState* state = stateManager.GetState(player->address);
                if (!state || (GetTickCount64() - state->deathTimestamp) > CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS) {
                    continue; // Animation is over (or never started), so hide the player now.
                }
                // Otherwise, keep the player in the list so the animation can render.
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
            
            // Apply health filter
            bool isAlive = npc->currentHealth > 0.0f;
            if (!isAlive && !settings.npcESP.showDeadNpcs) {
                // This NPC is dead, and the user wants to hide dead NPCs.
                // EXCEPTION: Keep it if the death animation is still playing.
                const EntityCombatState* state = stateManager.GetState(npc->address);
                if (!state || (GetTickCount64() - state->deathTimestamp) > CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS) {
                    continue; // Animation is over (or never started), so hide it.
                }
                // Otherwise, we fall through and let it render so the animation can play.
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