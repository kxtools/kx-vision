#include "CombatObservationTracker.h"

#include "CombatObservationLogic.h"

#include <cmath>

namespace kx {

void CombatObservationTracker::Update(const std::vector<GameEntity*>& entities, uint64_t now) {
    for (const auto* entity : entities) {
        if (!entity || !entity->isValid ||
            !std::isfinite(entity->currentHealth) ||
            !std::isfinite(entity->maxHealth) || entity->maxHealth <= 0.0f ||
            !std::isfinite(entity->currentBarrier) ||
            !std::isfinite(entity->position.x) ||
            !std::isfinite(entity->position.y) ||
            !std::isfinite(entity->position.z)) {
            continue;
        }

        const CombatObservationKey key = entity->GetCombatKey();
        if (!key.IsValid()) {
            continue;
        }
        CombatObservationLogic::Update(m_observations[key], *entity, now);
    }
}

void CombatObservationTracker::Prune(
    const ankerl::unordered_dense::set<CombatObservationKey, CombatObservationKeyHash>& activeKeys) {
    for (auto it = m_observations.begin(); it != m_observations.end();) {
        if (activeKeys.find(it->first) == activeKeys.end()) {
            it = m_observations.erase(it);
        } else {
            ++it;
        }
    }
}

void CombatObservationTracker::CopyObservationsTo(
    ankerl::unordered_dense::map<CombatObservationKey, CombatObservation, CombatObservationKeyHash>& out) const {
    out = m_observations;
}

void CombatObservationTracker::Clear() {
    m_observations.clear();
}

} // namespace kx
