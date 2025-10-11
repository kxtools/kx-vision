#pragma once

#include "../Data/ESPData.h"
#include "../../Game/Camera.h"

namespace kx {

    class CombatStateManager; // Forward declaration

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
                                 PooledFrameRenderData& filteredData, const CombatStateManager& stateManager, uint64_t now);

};

} // namespace kx