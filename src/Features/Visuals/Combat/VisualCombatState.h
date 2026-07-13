#pragma once

#include "TrailHistory.h"

#include <cstdint>

namespace kx {

struct VisualCombatState {
    float accumulatedDamage = 0.0f;
    float damageToDisplay = 0.0f;
    uint64_t burstStartTime = 0;
    uint64_t flushAnimationStartTime = 0;

    float healStartHealth = 0.0f;
    uint64_t lastVisualHealTimestamp = 0;

    float lastObservedDamageTotal = 0.0f;
    uint64_t lastDamageEventSequence = 0;
    uint64_t lastHealingEventSequence = 0;
    uint64_t lastPresentationResetSequence = 0;
    uint64_t lastTrailResetSequence = 0;
    uint64_t lastSeenSnapshotTimestamp = 0;

    TrailHistory trail;
};

} // namespace kx
