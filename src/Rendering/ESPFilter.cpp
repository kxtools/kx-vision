#include "ESPFilter.h"
#include "ESPMath.h"
#include <limits>
#include <cmath>

namespace kx {

bool ESPFilter::IsWithinDistanceLimit(const glm::vec3& entityPos, const glm::vec3& cameraPos, 
                                     bool useDistanceLimit, float distanceLimit) {
    if (!useDistanceLimit) {
        return true; // No distance limiting
    }
    
    // Use squared distance comparison to avoid expensive sqrt calculation
    const glm::vec3 deltaPos = entityPos - cameraPos;
    const float distanceSquared = glm::dot(deltaPos, deltaPos);
    const float limitSquared = distanceLimit * distanceLimit;
    
    return distanceSquared <= limitSquared;
}

bool ESPFilter::IsHealthValid(float currentHealth) {
    // Filter out dead entities (0 HP or negative HP)
    return currentHealth > 0.0f;
}

void ESPFilter::FilterPooledData(const PooledFrameRenderData& extractedData, Camera& camera,
                                 PooledFrameRenderData& filteredData) {
    filteredData.Reset();
    
    const auto& settings = AppState::Get().GetSettings();
    const glm::vec3 cameraPos = camera.GetPlayerPosition();
    
    // Filter players - apply same logic but work with pointers
    if (settings.playerESP.enabled) {
        filteredData.players.reserve(extractedData.players.size());
        for (RenderablePlayer* player : extractedData.players) {
            if (!player || !player->isValid) continue;
            
            // Apply local player filter
            if (player->isLocalPlayer && !settings.playerESP.showLocalPlayer) continue;
            
            // Apply health filter
            if (!IsHealthValid(player->currentHealth)) continue;
            
            // Apply distance filter
            if (!IsWithinDistanceLimit(player->position, cameraPos, 
                                       settings.espUseDistanceLimit, settings.espRenderDistanceLimit)) continue;
            
            // Calculate distance for rendering
            player->distance = glm::length(player->position - cameraPos);
            
            filteredData.players.push_back(player);
        }
    }
    
    // Filter NPCs - apply same logic but work with pointers
    if (settings.npcESP.enabled) {
        filteredData.npcs.reserve(extractedData.npcs.size());
        for (RenderableNpc* npc : extractedData.npcs) {
            if (!npc || !npc->isValid) continue;
            
            // Apply health filter
            if (!IsHealthValid(npc->currentHealth)) continue;
            
            // Apply distance filter
            if (!IsWithinDistanceLimit(npc->position, cameraPos, 
                                       settings.espUseDistanceLimit, settings.espRenderDistanceLimit)) continue;
            
            // Apply attitude-based filter
            if (!Filtering::EntityFilter::ShouldRenderNpc(npc->attitude, settings.npcESP)) continue;
            
            // Calculate distance for rendering
            npc->distance = glm::length(npc->position - cameraPos);
            
            filteredData.npcs.push_back(npc);
        }
    }
    
    // Filter gadgets - apply same logic but work with pointers
    if (settings.objectESP.enabled) {
        filteredData.gadgets.reserve(extractedData.gadgets.size());
        for (RenderableGadget* gadget : extractedData.gadgets) {
            if (!gadget || !gadget->isValid) continue;
            
            // Apply distance filter
            if (!IsWithinDistanceLimit(gadget->position, cameraPos, 
                                       settings.espUseDistanceLimit, settings.espRenderDistanceLimit)) continue;
            
            // Apply gadget type-based filter
            if (!Filtering::EntityFilter::ShouldRenderGadget(gadget->type, settings.objectESP)) continue;
            
            // Calculate distance for rendering
            gadget->distance = glm::length(gadget->position - cameraPos);
            
            filteredData.gadgets.push_back(gadget);
        }
    }
}

} // namespace kx