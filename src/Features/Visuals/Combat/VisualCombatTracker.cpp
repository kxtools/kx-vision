#include "VisualCombatTracker.h"

#include "VisualCombatConstants.h"
#include "../../../Game/Data/EntityData.h"
#include "../../../Game/Data/FrameData.h"
#include "../../../Game/Services/Combat/CombatObservation.h"

#include <algorithm>
#include <cmath>

namespace kx {

namespace {
constexpr float MIN_POSITION_CHANGE = 0.1f;
}

void VisualCombatTracker::Update(const EntityFrameSnapshot& snapshot, size_t maxTrailPoints) {
    if (snapshot.capturedAtMs == m_lastSnapshotTimestamp) {
        return;
    }
    m_lastSnapshotTimestamp = snapshot.capturedAtMs;
    maxTrailPoints = (std::min)(maxTrailPoints, MAX_TRAIL_HISTORY_CAPACITY);

    const auto updateCollection = [&](const auto& collection) {
        for (const auto* entity : collection) {
            if (!entity) {
                continue;
            }
            const auto observationIt = snapshot.combatObservations.find(entity->GetCombatKey());
            if (observationIt == snapshot.combatObservations.end()) {
                continue;
            }
            UpdateEntity(*entity, observationIt->second, snapshot.capturedAtMs, maxTrailPoints);
        }
    };

    updateCollection(snapshot.frameData.players);
    updateCollection(snapshot.frameData.npcs);
    updateCollection(snapshot.frameData.gadgets);
    updateCollection(snapshot.frameData.attackTargets);
    updateCollection(snapshot.frameData.items);

    for (auto it = m_states.begin(); it != m_states.end();) {
        if (it->second.lastSeenSnapshotTimestamp != snapshot.capturedAtMs) {
            it = m_states.erase(it);
        } else {
            ++it;
        }
    }
}

void VisualCombatTracker::UpdateEntity(
    const GameEntity& entity,
    const CombatObservation& observation,
    uint64_t capturedAtMs,
    size_t maxTrailPoints) {
    auto [it, inserted] = m_states.try_emplace(entity.GetCombatKey());
    VisualCombatState& state = it->second;
    state.lastSeenSnapshotTimestamp = capturedAtMs;
    state.trail.Limit(maxTrailPoints);

    if (inserted) {
        state.lastObservedDamageTotal = observation.totalDamageObserved;
        state.lastDamageEventSequence = observation.damageEventSequence;
        state.lastHealingEventSequence = observation.healingEventSequence;
        state.lastPresentationResetSequence = observation.presentationResetSequence;
        state.lastTrailResetSequence = observation.trailResetSequence;
        state.lastVisualHealTimestamp = observation.lastHealTimestamp;
        state.healStartHealth = observation.healthBeforeLastHeal;
    } else {
        if (observation.presentationResetSequence != state.lastPresentationResetSequence) {
            state.accumulatedDamage = 0.0f;
            state.damageToDisplay = 0.0f;
            state.burstStartTime = 0;
            state.flushAnimationStartTime = 0;
            state.lastObservedDamageTotal = observation.damageTotalAtLastReset;
            state.lastPresentationResetSequence = observation.presentationResetSequence;
        }
        if (observation.trailResetSequence != state.lastTrailResetSequence) {
            state.trail = {};
            state.healStartHealth = observation.healthBeforeLastHeal;
            state.lastVisualHealTimestamp = observation.lastHealTimestamp;
            state.lastHealingEventSequence = observation.healingEventSequence;
            state.lastTrailResetSequence = observation.trailResetSequence;
        }

        if (state.flushAnimationStartTime > 0 &&
            CombatElapsedMs(capturedAtMs, state.flushAnimationStartTime) >=
                VisualCombatEffects::DAMAGE_ACCUMULATOR_FADE_MS) {
            state.accumulatedDamage = 0.0f;
            state.damageToDisplay = 0.0f;
            state.flushAnimationStartTime = 0;
        }

        if (observation.damageEventSequence != state.lastDamageEventSequence) {
            const float newDamage = observation.totalDamageObserved - state.lastObservedDamageTotal;
            if (newDamage > 0.0f && (std::isfinite)(newDamage)) {
                if (state.accumulatedDamage <= 0.0f) {
                    state.burstStartTime = observation.lastHitTimestamp;
                }
                state.accumulatedDamage += newDamage;
            }
            state.lastObservedDamageTotal = observation.totalDamageObserved;
            state.lastDamageEventSequence = observation.damageEventSequence;
        }

        if (observation.healingEventSequence != state.lastHealingEventSequence) {
            if (CombatElapsedMs(observation.lastHealTimestamp, state.lastVisualHealTimestamp) >
                VisualCombatEffects::BURST_HEAL_WINDOW_MS) {
                state.healStartHealth = observation.healthBeforeLastHeal;
            }
            state.lastVisualHealTimestamp = observation.lastHealTimestamp;
            state.lastHealingEventSequence = observation.healingEventSequence;
        }
    }

    if (state.flushAnimationStartTime == 0 && state.accumulatedDamage > 0.0f) {
        const bool deathFlush = observation.deathTimestamp > 0 &&
            CombatElapsedMs(capturedAtMs, observation.lastHitTimestamp) >
                VisualCombatEffects::POST_MORTEM_FLUSH_DELAY_MS;
        const bool inactivityFlush = observation.deathTimestamp == 0 &&
            CombatElapsedMs(capturedAtMs, observation.lastHitTimestamp) >
                VisualCombatEffects::BURST_INACTIVITY_TIMEOUT_MS;
        const bool durationFlush = observation.deathTimestamp == 0 && state.burstStartTime > 0 &&
            CombatElapsedMs(capturedAtMs, state.burstStartTime) >
                VisualCombatEffects::MAX_BURST_DURATION_MS;
        if (deathFlush || inactivityFlush || durationFlush) {
            state.flushAnimationStartTime = capturedAtMs;
            state.damageToDisplay = state.accumulatedDamage;
        }
    }

    bool recordPosition = state.trail.size == 0;
    if (!recordPosition) {
        recordPosition = glm::distance(entity.position, state.trail.Get(state.trail.size - 1).position) >=
            MIN_POSITION_CHANGE;
    }
    if (recordPosition) {
        state.trail.Push({entity.position, capturedAtMs}, maxTrailPoints);
    }
}

void VisualCombatTracker::Reset() {
    m_states.clear();
    m_lastSnapshotTimestamp = 0;
}

} // namespace kx
