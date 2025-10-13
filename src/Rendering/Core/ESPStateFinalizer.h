#pragma once

#include "../Data/ESPData.h"

// Forward declarations
struct FrameContext;

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
         * @param context The frame context.
         * @param finalizedData The data that has been processed by the ESPVisualsProcessor.
         */
        static void Finalize(const FrameContext& context, const PooledFrameRenderData& finalizedData);
    };
} // namespace kx