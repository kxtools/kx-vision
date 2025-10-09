#pragma once
#include <cstdint>

namespace kx
{
	// Holds the dynamic combat information for a single entity
	struct EntityCombatState
	{
		// Last observed health snapshot
		float lastKnownHealth = 0.0f;

		// Damage event data
		float lastDamageTaken = 0.0f;
		uint64_t lastHitTimestamp = 0;

		// Healing event data
		float healStartHealth = 0.0f;
		uint64_t lastHealTimestamp = 0;
		uint64_t lastHealFlashTimestamp = 0;

		// Lifecycle
		uint64_t deathTimestamp = 0; // When health first reached zero
		uint64_t lastSeenTimestamp = 0; // Last frame we processed this entity

		// Accumulated damage behavior
		float accumulatedDamage = 0.0f;
		uint64_t flushAnimationStartTime = 0; // Tracks the start of the fade-out animation.

		// Utility helpers (optional future use)
		bool IsDead() const { return deathTimestamp != 0; }
		bool HasAccumulatedDamage() const { return accumulatedDamage > 0.0f; }
	};
} // namespace kx
