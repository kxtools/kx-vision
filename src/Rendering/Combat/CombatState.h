#pragma once
#include <cstdint>

namespace kx {
    // Holds the dynamic combat information for a single entity
    struct EntityCombatState {
        float lastKnownHealth = 0.0f;
        float lastDamageTaken = 0.0f;
        uint64_t lastHitTimestamp = 0;  // Time of the last detected damage event
        
        // --- NEW MEMBERS FOR HEALING ---
        float healStartHealth = 0.0f;   // Health value before the heal started
        uint64_t lastHealTimestamp = 0; // Timestamp of the last healing event
        uint64_t lastHealFlashTimestamp = 0; // Timestamp for the FAST flash effect
        uint64_t deathTimestamp = 0; // Timestamp for when health first hit zero

        uint64_t lastSeenTimestamp = 0; // Time the entity was last seen in a frame

        // --- NEW MEMBERS FOR DAMAGE ACCUMULATION ---
        float accumulatedDamage = 0.0f;     // Stores damage received since the last flush.
        uint64_t lastFlushTimestamp = 0;    // Tracks the time of the last flush event.
    };
}
