#pragma once

#include "glm.hpp"
#include <vector>
#include <cstdint>
#include <memory>
#include <ankerl/unordered_dense.h>
#include "../Services/Combat/CombatObservation.h"
#include "../Services/Combat/CombatObservationKey.h"

namespace kx {
    struct GameEntity;
    struct PlayerEntity;
    struct NpcEntity;
    struct GadgetEntity;
    struct AttackTargetEntity;
    struct ItemEntity;
}

namespace kx {

struct FrameGameData {
    std::vector<PlayerEntity*> players;
    std::vector<NpcEntity*> npcs;
    std::vector<GadgetEntity*> gadgets;
    std::vector<AttackTargetEntity*> attackTargets;
    std::vector<ItemEntity*> items;

    ankerl::unordered_dense::map<uint32_t, GameEntity*> entityMap;

    void Reset() {
        players.clear();
        npcs.clear();
        gadgets.clear();
        attackTargets.clear();
        items.clear();
        entityMap.clear();
    }

    template <typename T = GameEntity>
    const T* GetByID(uint32_t id) const {
        auto it = entityMap.find(id);
        if (it != entityMap.end()) {
            return static_cast<const T*>(it->second);
        }
        return nullptr;
    }
};

struct EntityFrameSnapshot {
    FrameGameData frameData;
    ankerl::unordered_dense::map<CombatObservationKey, CombatObservation, CombatObservationKeyHash> combatObservations;
    // Monotonic GetTickCount64 capture time. Not Unix/wall time and not game simulation time.
    uint64_t capturedAtMs = 0;

    const CombatObservation* GetCombatObservation(CombatObservationKey key) const {
        const auto it = combatObservations.find(key);
        return it != combatObservations.end() ? &it->second : nullptr;
    }
};

// Guardrail: FrameGameData contains pointers into snapshot-owned pools. Consumers must
// retain this handle for every use; copying FrameGameData does not extend pointer lifetime.
using FrameSnapshotHandle = std::shared_ptr<const EntityFrameSnapshot>;

} // namespace kx
