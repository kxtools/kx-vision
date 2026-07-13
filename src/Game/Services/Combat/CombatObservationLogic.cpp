#include "CombatObservationLogic.h"

#include "CombatObservation.h"
#include "../../Data/EntityData.h"

#include <cmath>

namespace kx {

void CombatObservationLogic::Update(CombatObservation& observation, const GameEntity& entity, uint64_t now) {
    if (HandleAttributeChanges(entity, observation, now)) {
        return;
    }

    UpdateBarrier(entity, observation, now);
    ProcessHealthChanges(entity, observation, now);

    observation.lastKnownHealth = entity.currentHealth;
    observation.lastKnownMaxHealth = entity.maxHealth;
    observation.lastKnownBarrier = entity.currentBarrier;
    observation.lastSeenTimestamp = now;
}

bool CombatObservationLogic::HandleAttributeChanges(
    const GameEntity& entity,
    CombatObservation& observation,
    uint64_t now) {
    if (observation.lastKnownMaxHealth > 0.0f &&
        std::fabs(entity.maxHealth - observation.lastKnownMaxHealth) > 1.0f) {
        observation.lastKnownHealth = entity.currentHealth;
        observation.lastKnownMaxHealth = entity.maxHealth;
        observation.lastKnownBarrier = entity.currentBarrier;
        observation.lastSeenTimestamp = now;
        if (entity.currentHealth > 0.0f) {
            observation.deathTimestamp = 0;
            ++observation.presentationResetSequence;
            observation.damageTotalAtLastReset = observation.totalDamageObserved;
        }
        return true;
    }

    if (entity.entityType == EntityTypes::Gadget &&
        observation.lastKnownMaxHealth > 0.0f &&
        observation.lastKnownHealth >= observation.lastKnownMaxHealth &&
        entity.currentHealth <= 0.0f) {
        ResetForRespawn(observation, entity.currentHealth, now);
        return true;
    }

    return false;
}

void CombatObservationLogic::ResetForRespawn(
    CombatObservation& observation,
    float currentHealth,
    uint64_t now) {
    // Event counters stay monotonic for this entity key so snapshot consumers can
    // detect every newly observed transition without owning game-thread state.
    const float totalDamageObserved = observation.totalDamageObserved;
    const uint64_t damageEventSequence = observation.damageEventSequence;
    const uint64_t healingEventSequence = observation.healingEventSequence;
    const uint64_t presentationResetSequence = observation.presentationResetSequence + 1;
    const uint64_t trailResetSequence = observation.trailResetSequence + 1;
    const float currentBarrier = observation.lastKnownBarrier;

    observation = {};
    observation.lastKnownHealth = currentHealth;
    observation.lastKnownBarrier = currentBarrier;
    observation.lastSeenTimestamp = now;
    observation.totalDamageObserved = totalDamageObserved;
    observation.damageEventSequence = damageEventSequence;
    observation.healingEventSequence = healingEventSequence;
    observation.presentationResetSequence = presentationResetSequence;
    observation.trailResetSequence = trailResetSequence;
    observation.damageTotalAtLastReset = totalDamageObserved;
}

void CombatObservationLogic::UpdateBarrier(
    const GameEntity& entity,
    CombatObservation& observation,
    uint64_t now) {
    if (entity.currentBarrier != observation.lastKnownBarrier) {
        observation.barrierOnLastChange = observation.lastKnownBarrier;
        observation.lastBarrierChangeTimestamp = now;
    }
}

void CombatObservationLogic::ProcessHealthChanges(
    const GameEntity& entity,
    CombatObservation& observation,
    uint64_t now) {
    if (observation.lastSeenTimestamp == 0) {
        return;
    }

    if (entity.currentHealth < observation.lastKnownHealth) {
        const float damage = observation.lastKnownHealth - entity.currentHealth;
        if (damage <= 0.0f) {
            return;
        }
        observation.lastDamageTaken = damage;
        observation.totalDamageObserved += damage;
        ++observation.damageEventSequence;
        observation.lastHitTimestamp = now;
        if (entity.currentHealth <= 0.0f && observation.deathTimestamp == 0) {
            observation.deathTimestamp = now;
        }
        return;
    }

    if (entity.currentHealth > observation.lastKnownHealth) {
        if (observation.lastKnownHealth <= 0.0f) {
            ResetForRespawn(observation, entity.currentHealth, now);
            return;
        }
        observation.healthBeforeLastHeal = observation.lastKnownHealth;
        ++observation.healingEventSequence;
        observation.lastHealTimestamp = now;
    }
}

} // namespace kx
