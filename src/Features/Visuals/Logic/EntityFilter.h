#pragma once

#include "../../../Game/Data/FrameData.h"

namespace kx {

    class CombatStateManager; // Forward declaration

class EntityFilter {
public:
    /**
     * @brief OPTIMIZED filter method - filters already pooled data (no object allocations)
     * @param extractedData Input pooled data from extraction
     * @param context Frame context containing camera, settings, game state, etc.
     * @param filteredData Output filtered pooled data
     */
    static void FilterPooledData(const FrameGameData& extractedData, const FrameContext& context,
                                 FrameGameData& filteredData);

};

} // namespace kx