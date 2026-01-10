#pragma once

#include <vector>
#include <ankerl/unordered_dense.h>
#include "../Data/EntityData.h"
#include "../Data/FrameData.h"
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
    class DataExtractor {
    public:
        /**
         * @brief OPTIMIZED extraction method - extracts directly into object pools (eliminates heap allocations)
         * @param playerPool Object pool for players
         * @param npcPool Object pool for NPCs
         * @param gadgetPool Object pool for gadgets
         * @param attackTargetPool Object pool for attack targets
         * @param itemPool Object pool for items
         * @param pooledData Output container for pooled data pointers
         * @param charToNameMap Persistent map for character-to-player-name lookup (cleared each frame, retains capacity)
         */
        static void ExtractFrameData(ObjectPool<PlayerEntity>& playerPool,
            ObjectPool<NpcEntity>& npcPool,
            ObjectPool<GadgetEntity>& gadgetPool,
            ObjectPool<AttackTargetEntity>& attackTargetPool,
            ObjectPool<ItemEntity>& itemPool,
            FrameGameData& pooledData,
            ankerl::unordered_dense::map<void*, const wchar_t*>& charToNameMap);

    private:
        /**
         * @brief OPTIMIZED extraction methods - write directly into object pools
         */
        static void ExtractCharacterData(ObjectPool<PlayerEntity>& playerPool,
            ObjectPool<NpcEntity>& npcPool,
            std::vector<PlayerEntity*>& players,
            std::vector<NpcEntity*>& npcs,
            const ankerl::unordered_dense::map<void*, const wchar_t*>& characterToPlayerNameMap);

        static void ExtractGadgetData(ObjectPool<GadgetEntity>& gadgetPool,
            std::vector<GadgetEntity*>& gadgets);

        static void ExtractAttackTargetData(ObjectPool<AttackTargetEntity>& attackTargetPool,
            std::vector<AttackTargetEntity*>& attackTargets);

        static void ExtractItemData(ObjectPool<ItemEntity>& itemPool,
            std::vector<ItemEntity*>& items);
    };

} // namespace kx