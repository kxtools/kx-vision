#pragma once

#include "RenderableData.h"
#include "../Core/AppState.h"
#include "../Utils/EntityFilter.h"
#include "../Game/Camera.h"
#include <glm.hpp>

namespace kx {

/**
 * @brief Filtering stage for the ESP rendering pipeline
 * 
 * This class operates on raw FrameRenderData from ESPDataExtractor and applies
 * all user-configurable filters to produce a smaller, filtered dataset for rendering.
 * 
 * Responsibilities:
 * - Distance-based culling
 * - Settings-based filtering (enabled/disabled categories)
 * - Entity-specific filtering (attitudes, gadget types, etc.)
 * - Health-based filtering (dead entities)
 * - Local player filtering
 * - Priority-based sorting (optional)
 */
class ESPFilter {
public:
    /**
     * @brief Apply all filters to frame data
     * @param rawData Input data from ESPDataExtractor
     * @param camera Camera for distance calculations
     * @param filteredData Output filtered data ready for rendering
     */
    static void FilterFrameData(const FrameRenderData& rawData, Camera& camera, FrameRenderData& filteredData);

private:
    /**
     * @brief Filter player entities
     * @param rawPlayers Input player data
     * @param camera Camera for distance calculations
     * @param filteredPlayers Output filtered player data
     */
    static void FilterPlayers(const std::vector<RenderablePlayer>& rawPlayers, Camera& camera, 
                             std::vector<RenderablePlayer>& filteredPlayers);

    /**
     * @brief Filter NPC entities
     * @param rawNpcs Input NPC data
     * @param camera Camera for distance calculations
     * @param filteredNpcs Output filtered NPC data
     */
    static void FilterNpcs(const std::vector<RenderableNpc>& rawNpcs, Camera& camera, 
                          std::vector<RenderableNpc>& filteredNpcs);

    /**
     * @brief Filter gadget entities
     * @param rawGadgets Input gadget data
     * @param camera Camera for distance calculations
     * @param filteredGadgets Output filtered gadget data
     */
    static void FilterGadgets(const std::vector<RenderableGadget>& rawGadgets, Camera& camera, 
                             std::vector<RenderableGadget>& filteredGadgets);

    /**
     * @brief Check if entity is within distance limits
     * @param entityPos Entity position
     * @param cameraPos Camera position
     * @param useDistanceLimit Whether distance limiting is enabled
     * @param distanceLimit Maximum distance
     * @return true if entity should be rendered based on distance
     */
    static bool IsWithinDistanceLimit(const glm::vec3& entityPos, const glm::vec3& cameraPos, 
                                     bool useDistanceLimit, float distanceLimit);

    /**
     * @brief Check if entity has valid health (not dead)
     * @param currentHealth Entity's current health
     * @return true if entity should be rendered based on health
     */
    static bool IsHealthValid(float currentHealth);
};

} // namespace kx