#pragma once

#include "VisualCombatState.h"
#include "../../../Game/Services/Combat/CombatObservationKey.h"

#include <ankerl/unordered_dense.h>
#include <cstddef>
#include <cstdint>

namespace kx {

struct CombatObservation;
struct EntityFrameSnapshot;
struct GameEntity;

class VisualCombatTracker {
public:
    using StateMap = ankerl::unordered_dense::map<
        CombatObservationKey,
        VisualCombatState,
        CombatObservationKeyHash>;

    void Update(const EntityFrameSnapshot& snapshot, size_t maxTrailPoints);
    void Reset();

    [[nodiscard]] const StateMap& GetStates() const noexcept { return m_states; }

private:
    void UpdateEntity(
        const GameEntity& entity,
        const CombatObservation& observation,
        uint64_t capturedAtMs,
        size_t maxTrailPoints);

    StateMap m_states;
    uint64_t m_lastSnapshotTimestamp = 0;
};

} // namespace kx
