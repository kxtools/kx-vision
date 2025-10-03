#pragma once

#include <vector>
#include <unordered_map>
#include "../Data/RenderableData.h"
#include "../Data/ESPData.h"
#include "../../Utils/ObjectPool.h"

namespace kx {

    /**
     * @brief Handles data extraction from game memory (Stage 1 of rendering pipeline)
     *
     * This class encapsulates all unsafe memory operations that read from game structures.
     * It extracts data into safe local data structures that can be rendered without
     * risk of memory access violations.
     *
     * Performance Optimization:
     * - Implements fail-fast validation of root ContextCollection pointer
     * - Prevents thousands of failed memory reads during loading screens or when game is not ready
     */
    class ESPDataExtractor {
    public:
        /**
         * @brief OPTIMIZED extraction method - extracts directly into object pools (eliminates heap allocations)
         * @param playerPool Object pool for players
         * @param npcPool Object pool for NPCs
         * @param gadgetPool Object pool for gadgets
         * @param pooledData Output container for pooled data pointers
         */
        static void ExtractFrameData(ObjectPool<RenderablePlayer>& playerPool,
            ObjectPool<RenderableNpc>& npcPool,
            ObjectPool<RenderableGadget>& gadgetPool,
            PooledFrameRenderData& pooledData);

    private:
        /**
         * @brief OPTIMIZED extraction methods - write directly into object pools
         *        Now also updates persistent entity state for interpolation
         */
        static void ExtractCharacterData(ObjectPool<RenderablePlayer>& playerPool,
            ObjectPool<RenderableNpc>& npcPool,
            std::vector<RenderablePlayer*>& players,
            std::vector<RenderableNpc*>& npcs,
            const std::unordered_map<void*, const wchar_t*>& characterToPlayerNameMap,
            std::unordered_map<const void*, RenderablePlayer>& persistentPlayers,
            std::unordered_map<const void*, RenderableNpc>& persistentNpcs);

        static void ExtractGadgetData(ObjectPool<RenderableGadget>& gadgetPool,
            std::vector<RenderableGadget*>& gadgets,
            std::unordered_map<const void*, RenderableGadget>& persistentGadgets);
    };

} // namespace kx