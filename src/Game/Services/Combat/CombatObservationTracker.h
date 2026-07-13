#pragma once

#include "CombatObservation.h"
#include "CombatObservationKey.h"
#include "../../Data/EntityData.h"

#include <ankerl/unordered_dense.h>
#include <vector>

namespace kx {

// Game-thread owner for neutral combat observations. Render receives snapshot copies only.
class CombatObservationTracker {
public:
    void Update(const std::vector<GameEntity*>& entities, uint64_t now);
    void Prune(const ankerl::unordered_dense::set<CombatObservationKey, CombatObservationKeyHash>& activeKeys);
    void CopyObservationsTo(
        ankerl::unordered_dense::map<CombatObservationKey, CombatObservation, CombatObservationKeyHash>& out) const;
    void Clear();

private:
    ankerl::unordered_dense::map<CombatObservationKey, CombatObservation, CombatObservationKeyHash> m_observations;
};

} // namespace kx
