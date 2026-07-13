#pragma once

#include <cstdint>

namespace kx {

[[nodiscard]] constexpr uint64_t CombatElapsedMs(uint64_t now, uint64_t timestamp) noexcept {
    // Guardrail: combat timestamps share one monotonic GetTickCount64 domain. Clock
    // reversal must not become a huge unsigned elapsed duration.
    return now >= timestamp ? now - timestamp : 0;
}

// Neutral facts observed from successive game-thread entity snapshots.
struct CombatObservation {
    float lastKnownHealth = 0.0f;
    float lastKnownMaxHealth = 0.0f;
    float lastKnownBarrier = 0.0f;

    // Neutral reset markers let snapshot consumers discard derived state without
    // importing presentation policy into game-thread observation code.
    uint64_t presentationResetSequence = 0;
    uint64_t trailResetSequence = 0;
    float damageTotalAtLastReset = 0.0f;

    float lastDamageTaken = 0.0f;
    float totalDamageObserved = 0.0f;
    uint64_t damageEventSequence = 0;
    uint64_t lastHitTimestamp = 0;

    float healthBeforeLastHeal = 0.0f;
    uint64_t healingEventSequence = 0;
    uint64_t lastHealTimestamp = 0;

    uint64_t deathTimestamp = 0;
    uint64_t lastSeenTimestamp = 0;

    float barrierOnLastChange = 0.0f;
    uint64_t lastBarrierChangeTimestamp = 0;

    [[nodiscard]] bool IsDead() const noexcept { return deathTimestamp != 0; }
};

} // namespace kx
