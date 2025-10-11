#pragma once

#include "../Data/ESPData.h"

// Forward declarations
namespace kx {
    class Camera;
    class CombatStateManager;
}

namespace kx {
    /**
     * @brief Handles the final stage of state processing before rendering.
     *
     * This class is responsible for running any calculations that depend on the final
     * on-screen layout of entities (e.g., health bar width). It bridges the gap
     * between filtering and rendering, ensuring the CombatStateManager is fully updated
     * before any drawing occurs.
     */
    class ESPStateFinalizer {
    public:
        /**
         * @brief Processes filtered data to finalize all combat state animations.
         * @param filteredData The data that has passed through the ESPFilter.
         * @param camera The camera reference for visual calculations.
         * @param stateManager The combat state manager to be updated.
         * @param now The current timestamp for the frame.
         */
        static void Finalize(
            const PooledFrameRenderData& filteredData,
            Camera& camera,
            CombatStateManager& stateManager,
            uint64_t now
        );
    };
} // namespace kx