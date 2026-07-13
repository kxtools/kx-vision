#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>
#include <array>
#include <ankerl/unordered_dense.h>
#include "../../Game/Data/FrameData.h"
#include "../../Game/Data/EntityData.h"
#include "../../Game/Data/EntityLimits.h"
#include "../../Utils/ObjectPool.h"
#include "../../Game/Services/Combat/CombatObservationTracker.h"
#include "../../Game/Services/Combat/CombatObservationKey.h"

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
     * Performs throttled extraction using feature-supplied cadence.
     * Updates object pools, extracts frame data, and manages combat states.
     * 
     * @param now Current time in milliseconds (from GetTickCount64)
     */
    void Update(uint64_t now);

    // Atomic handle pins both pointer lists and backing pools for full consumer lifetime.
    FrameSnapshotHandle AcquireFrameSnapshot() const {
        return m_publishedSnapshot.load(std::memory_order_acquire);
    }

    void SetUpdateRateHz(float rate) { m_requestedUpdateRateHz.store(rate, std::memory_order_relaxed); }

    /**
     * @brief Reset all pools and frame data (e.g., on map change)
     */
    void Reset();

private:
    static constexpr size_t BUFFER_COUNT = 3;
    struct SnapshotSlot final : EntityFrameSnapshot {
        ObjectPool<PlayerEntity> players{EntityLimits::MAX_PLAYERS};
        ObjectPool<NpcEntity> npcs{EntityLimits::MAX_NPCS};
        ObjectPool<GadgetEntity> gadgets{EntityLimits::MAX_GADGETS};
        ObjectPool<AttackTargetEntity> attackTargets{EntityLimits::MAX_ATTACK_TARGETS};
        ObjectPool<ItemEntity> items{EntityLimits::MAX_ITEMS};

        void ResetStorage() {
            players.Reset(); npcs.Reset(); gadgets.Reset(); attackTargets.Reset(); items.Reset();
            frameData.Reset();
        }
    };
    std::array<std::shared_ptr<SnapshotSlot>, BUFFER_COUNT> m_slots;
    std::atomic<FrameSnapshotHandle> m_publishedSnapshot;

    bool ShouldExtract(uint64_t now) const;
    std::shared_ptr<SnapshotSlot> FindWritableSlot();
    bool ExtractInto(SnapshotSlot& slot);
    void UpdateCombatObservations(SnapshotSlot& slot, uint64_t now);
    void PublishSnapshot(const std::shared_ptr<SnapshotSlot>& slot, uint64_t now);

    // Combat state management
    CombatObservationTracker m_combatObservationTracker;
    ankerl::unordered_dense::set<CombatObservationKey, CombatObservationKeyHash> m_activeCombatKeys;
    std::vector<GameEntity*> m_allEntitiesBuffer;

    // Persistent map for character-to-player-name lookup (cleared every frame, retains capacity)
    ankerl::unordered_dense::map<void*, const wchar_t*> m_charToNameMap;

    // Throttling
    uint64_t m_lastGameDataUpdateMs = 0;
    std::atomic<float> m_requestedUpdateRateHz{60.0f};
};

} // namespace kx
