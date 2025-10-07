#pragma once

#include "../Data/ESPData.h"
#include "../../Game/Camera.h"

namespace kx {

class CombatStateManager; // Forward declaration

/**
 * @brief Filtering stage for the ESP rendering pipeline
 * 
 * This class operates on pooled data from ESPDataExtractor and applies
 * all user-configurable filters to produce a smaller, filtered dataset for rendering.
 * 
 * Responsibilities:
 * - Hard distance-based culling (entities beyond user's specified limit)
 * - Settings-based filtering (enabled/disabled categories)
 * - Entity-specific filtering (attitudes, gadget types, etc.)
 * - Health-based filtering (configurable for dead entities)
 * - Local player filtering
 * 
 * Note: Visual effects like fading are handled by the renderer stage.
 * The filter only applies hard data limits based on user settings.
 */
class ESPFilter {
public:
    /**
     * @brief OPTIMIZED filter method - filters already pooled data (no object allocations)
     * @param extractedData Input pooled data from extraction
     * @param camera Camera for distance calculations
     * @param filteredData Output filtered pooled data
     * @param stateManager The combat state manager for state-aware filtering
     */
    static void FilterPooledData(const PooledFrameRenderData& extractedData, Camera& camera,
                                PooledFrameRenderData& filteredData, const CombatStateManager& stateManager);

private:
    /**
     * @brief Check if entity has valid health (configurable for dead entities)
     * @param currentHealth Entity's current health
     * @param showDeadEntities Whether to show entities with 0 HP
     * @return true if entity should be rendered based on health
     */
    static bool IsHealthValid(float currentHealth, bool showDeadEntities = false);

public:
    /**
     * @brief Calculate alpha value for distance-based fading
     * @param distance Actual distance to entity
     * @param useDistanceLimit Whether distance limiting is enabled
     * @param distanceLimit Maximum render distance
     * @return Alpha value from 0.0f (invisible) to 1.0f (fully visible)
     */
    static float CalculateDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit);
};

} // namespace kx