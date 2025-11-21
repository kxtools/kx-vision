#pragma once
#include <cstdint>
#include <array>
#include <glm/vec3.hpp>

namespace kx
{
	struct PositionHistoryPoint {
		glm::vec3 position;
		uint64_t timestamp;
	};

	constexpr size_t MAX_TRAIL_HISTORY_CAPACITY = 64;

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
		uint64_t burstStartTime = 0;          // Tracks when the current damage accumulation started.
		float damageToDisplay = 0.0f;
		
        // Barrier state
        float lastKnownBarrier = 0.0f;
        float barrierOnLastChange = 0.0f;
        uint64_t lastBarrierChangeTimestamp = 0;

        // Max health tracking for state change detection
        float lastKnownMaxHealth = 0.0f;

		// Movement trail history - fixed-size ring buffer
		std::array<PositionHistoryPoint, MAX_TRAIL_HISTORY_CAPACITY> positionHistory;
		size_t historyHead = 0;
		size_t historySize = 0;

		void PushHistory(const PositionHistoryPoint& p) {
			positionHistory[historyHead] = p;
			historyHead = (historyHead + 1) % MAX_TRAIL_HISTORY_CAPACITY;
			if (historySize < MAX_TRAIL_HISTORY_CAPACITY) {
				historySize++;
			}
		}

		const PositionHistoryPoint& GetHistoryItem(size_t index) const {
			size_t tail = (historyHead + MAX_TRAIL_HISTORY_CAPACITY - historySize) % MAX_TRAIL_HISTORY_CAPACITY;
			size_t actualIndex = (tail + index) % MAX_TRAIL_HISTORY_CAPACITY;
			return positionHistory[actualIndex];
		}

		// Utility helpers (optional future use)
		bool IsDead() const { return deathTimestamp != 0; }
		bool HasAccumulatedDamage() const { return accumulatedDamage > 0.0f; }
	};
} // namespace kx
