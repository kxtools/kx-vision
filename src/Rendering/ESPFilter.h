#pragma once

#include "RenderableData.h"
#include "ESPData.h"
#include "../Core/AppState.h"
#include "../Utils/EntityFilter.h"
#include "../Utils/ObjectPool.h"
#include "../Game/Camera.h"
#include <glm.hpp>

namespace kx {

/**
 * @brief Filtering stage for the ESP rendering pipeline
 * 
 * This class operates on pooled data from ESPDataExtractor and applies
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
     * @brief OPTIMIZED filter method - filters already pooled data (no object allocations)
     * @param extractedData Input pooled data from extraction
     * @param camera Camera for distance calculations
     * @param filteredData Output filtered pooled data
     */
    static void FilterPooledData(const PooledFrameRenderData& extractedData, Camera& camera,
                                PooledFrameRenderData& filteredData);

private:
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