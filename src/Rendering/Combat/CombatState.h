#pragma once
#include <cstdint>

namespace kx {
    // Holds the dynamic combat information for a single entity
    struct EntityCombatState {
        float lastKnownHealth = 0.0f;
        float lastDamageTaken = 0.0f;
        uint64_t lastHitTimestamp = 0;  // Time of the last detected damage event
        uint64_t lastHealTimestamp = 0; // Time of the last detected healing event
        uint64_t lastSeenTimestamp = 0; // Time the entity was last seen in a frame
    };
}
