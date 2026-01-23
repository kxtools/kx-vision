#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <array>
#include <ankerl/unordered_dense.h>
#include "../../Game/Data/FrameData.h"
#include "../../Game/Data/EntityData.h"
#include "../../Utils/ObjectPool.h"
#include "../../Rendering/Shared/LayoutConstants.h"
#include "../../Game/Services/Combat/CombatStateManager.h"
#include "../../Game/Services/Combat/CombatStateKey.h"

namespace kx {

/**
 * @brief Manages game entity data extraction, pooling, and combat state tracking
 * 
 * This class serves as the "Source of Truth" for all game entity data.
 * It handles:
 * - Object pooling for efficient entity management
 * - Throttled data extraction from game memory
 * - Combat state tracking and updates
 * - Frame data aggregation
 * 
 * Separated from AppLifecycleManager to enable clean separation of concerns
 */
class EntityManager {
public:
    EntityManager();
    ~EntityManager() = default;

    /**
     * @brief Update entity data extraction and combat states
     * 
     * Performs throttled extraction based on settings.espUpdateRate.
     * Updates object pools, extracts frame data, and manages combat states.
     * 
     * @param now Current time in milliseconds (from GetTickCount64)
     */
    void Update(uint64_t now);

    /**
     * @brief Get a copy of the current frame's game data.
     * Thread-safe: Returns by value so the Render Thread owns its own pointer lists;
     * the Game Thread can swap/reset without invalidating iterators.
     */
    FrameGameData GetFrameData() const {
        std::lock_guard<std::mutex> lock(m_snapshotMutex);
        return m_frameDataSnapshot;
    }

    /**
     * @brief Get reference to CombatStateManager for frame context
     * @return Reference to CombatStateManager instance
     */
    CombatStateManager& GetCombatStateManager() { return m_combatStateManager; }

    /**
     * @brief Reset all pools and frame data (e.g., on map change)
     */
    void Reset();

private:
    // Triple-buffered object pools: avoids Game Thread overwriting the pool
    // the Render Thread is reading when game FPS exceeds render FPS
    static constexpr size_t BUFFER_COUNT = 3;
    size_t m_writeIndex = 0;

    std::array<ObjectPool<PlayerEntity>, BUFFER_COUNT> m_playerPools{
        ObjectPool<PlayerEntity>(EntityLimits::MAX_PLAYERS),
        ObjectPool<PlayerEntity>(EntityLimits::MAX_PLAYERS),
        ObjectPool<PlayerEntity>(EntityLimits::MAX_PLAYERS)
    };
    std::array<ObjectPool<NpcEntity>, BUFFER_COUNT> m_npcPools{
        ObjectPool<NpcEntity>(EntityLimits::MAX_NPCS),
        ObjectPool<NpcEntity>(EntityLimits::MAX_NPCS),
        ObjectPool<NpcEntity>(EntityLimits::MAX_NPCS)
    };
    std::array<ObjectPool<GadgetEntity>, BUFFER_COUNT> m_gadgetPools{
        ObjectPool<GadgetEntity>(EntityLimits::MAX_GADGETS),
        ObjectPool<GadgetEntity>(EntityLimits::MAX_GADGETS),
        ObjectPool<GadgetEntity>(EntityLimits::MAX_GADGETS)
    };
    std::array<ObjectPool<AttackTargetEntity>, BUFFER_COUNT> m_attackTargetPools{
        ObjectPool<AttackTargetEntity>(EntityLimits::MAX_ATTACK_TARGETS),
        ObjectPool<AttackTargetEntity>(EntityLimits::MAX_ATTACK_TARGETS),
        ObjectPool<AttackTargetEntity>(EntityLimits::MAX_ATTACK_TARGETS)
    };
    std::array<ObjectPool<ItemEntity>, BUFFER_COUNT> m_itemPools{
        ObjectPool<ItemEntity>(EntityLimits::MAX_ITEMS),
        ObjectPool<ItemEntity>(EntityLimits::MAX_ITEMS),
        ObjectPool<ItemEntity>(EntityLimits::MAX_ITEMS)
    };

    // Double-buffering for thread-safe access
    // Work buffer: Populated by Game Thread during Update()
    FrameGameData m_frameDataWorkBuffer;
    // Snapshot: Stable copy for Render Thread consumption
    mutable FrameGameData m_frameDataSnapshot;
    mutable std::mutex m_snapshotMutex;

    // Combat state management
    CombatStateManager m_combatStateManager;
    ankerl::unordered_dense::set<CombatStateKey, CombatStateKeyHash> m_activeCombatKeys;
    std::vector<GameEntity*> m_allEntitiesBuffer;

    // Persistent map for character-to-player-name lookup (cleared every frame, retains capacity)
    ankerl::unordered_dense::map<void*, const wchar_t*> m_charToNameMap;

    // Throttling
    float m_lastGameDataUpdateTime = 0.0f;
};

} // namespace kx
