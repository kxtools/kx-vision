#include "ESPFilter.h"
#include "ESPMath.h"
#include <limits>
#include <cmath>

namespace kx {

bool ESPFilter::IsWithinExtendedDistanceLimit(const glm::vec3& entityPos, const glm::vec3& cameraPos, 
                                              bool useDistanceLimit, float distanceLimit) {
    if (!useDistanceLimit) {
        return true; // No distance limiting
    }
    
    // Calculate fade zone distance (e.g., for 90m limit, fade zone extends to ~100m)
    const float fadeZoneDistance = distanceLimit * FADE_ZONE_PERCENTAGE;
    const float extendedLimit = distanceLimit + fadeZoneDistance;
    
    // Use squared distance comparison to avoid expensive sqrt calculation
    const glm::vec3 deltaPos = entityPos - cameraPos;
    const float distanceSquared = glm::dot(deltaPos, deltaPos);
    const float extendedLimitSquared = extendedLimit * extendedLimit;
    
    return distanceSquared <= extendedLimitSquared;
}

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

float ESPFilter::CalculateDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    if (!useDistanceLimit) {
        return 1.0f; // Fully visible when no distance limit
    }
    
    // Calculate fade zone distances
    const float fadeZoneDistance = distanceLimit * FADE_ZONE_PERCENTAGE;
    const float fadeStartDistance = distanceLimit - fadeZoneDistance; // e.g., 80m for 90m limit
    const float fadeEndDistance = distanceLimit; // e.g., 90m for 90m limit
    
    if (distance <= fadeStartDistance) {
        return 1.0f; // Fully visible
    } else if (distance >= fadeEndDistance) {
        return 0.0f; // Fully transparent (should be culled in filter)
    } else {
        // Linear interpolation in fade zone
        const float fadeProgress = (distance - fadeStartDistance) / fadeZoneDistance;
        return 1.0f - fadeProgress; // Fade from 1.0 to 0.0
    }
}

bool ESPFilter::IsHealthValid(float currentHealth, bool showDeadEntities) {
    // If showing dead entities is enabled, accept all health values
    if (showDeadEntities) {
        return true;
    }
    
    // Otherwise, filter out dead entities (0 HP or negative HP)
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
            
            // Apply distance filter (includes fade zone)
            if (!IsWithinExtendedDistanceLimit(player->position, cameraPos, 
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
            if (!IsHealthValid(npc->currentHealth, settings.npcESP.showDeadNpcs)) continue;
            
            // Apply distance filter (includes fade zone)
            if (!IsWithinExtendedDistanceLimit(npc->position, cameraPos, 
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
            
            // Apply distance filter (includes fade zone)
            if (!IsWithinExtendedDistanceLimit(gadget->position, cameraPos, 
                                             settings.espUseDistanceLimit, settings.espRenderDistanceLimit)) continue;
            
            // Apply gadget type-based filter
            if (!Filtering::EntityFilter::ShouldRenderGadget(gadget->type, settings.objectESP)) continue;
            
            // Apply depleted resource node filter
            if (settings.hideDepletedNodes && gadget->type == Game::GadgetType::ResourceNode && !gadget->isGatherable) {
                continue;
            }
            
            // Calculate distance for rendering
            gadget->distance = glm::length(gadget->position - cameraPos);
            
            filteredData.gadgets.push_back(gadget);
        }
    }
}

} // namespace kx