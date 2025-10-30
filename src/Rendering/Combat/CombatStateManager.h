#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "CombatState.h"
#include "../Data/RenderableData.h" // For RenderableEntity

namespace kx
{
	/**
	 * @brief Tracks transient combat-related state (damage, healing, death, respawn) for render effects.
	 *
	 * Responsibilities:
	 *  - Detect damage bursts & accumulate them for "pending damage" overlays.
	 *  - Detect heals and provide timing for overlay/flash effects.
	 *  - Detect death & respawn transitions.
	 *  - Cleanup inactive entity states after a timeout.
	 *
	 * Thread-safety: NOT thread-safe. All methods are expected to be called from the render/game thread.
	 */
	class CombatStateManager
	{
	public:
		/**
		 * @brief Update/refresh state for a set of entities.
		 * @param entities Entities currently visible/processed this frame.
		 * @param now Current timestamp in milliseconds (use GetTickCount64 or equivalent).
		 */
		void Update(const std::vector<RenderableEntity*>& entities, uint64_t now);

		/**
		 * @brief Remove combat state for entities that are no longer present in the game.
		 * @param activeEntities Set of entity addresses that are currently active in the game.
		 */
		void Prune(const std::unordered_set<const void*>& activeEntities);

		/**
		 * @brief Get immutable pointer to stored entity combat state (nullptr if missing).
		 */
		const EntityCombatState* GetState(const void* entityId) const;

	private:
		EntityCombatState* GetStateNonConst(const void* entityId); // This can be made private now
		std::unordered_map<const void*, EntityCombatState> m_entityStates;

		// --- Internal helpers (all assume non-null entity & validity already checked) ---

		EntityCombatState& AcquireState(const RenderableEntity* entity);

		void ProcessEntity(RenderableEntity* entity, uint64_t now);
		void HandleDamage(EntityCombatState& state,
		                  const RenderableEntity* entity,
		                  float currentHealth,
		                  uint64_t now);
		void HandleHealing(EntityCombatState& state,
		                   const RenderableEntity* entity,
		                   float currentHealth,
		                   uint64_t now);
		void ResetForRespawn(EntityCombatState& state,
		                     float currentHealth,
		                     uint64_t now);

        // New helper methods for ProcessEntity refactoring
        void UpdateDamageAccumulatorAnimation(EntityCombatState& state, uint64_t now);
        bool DetectStateChangeOrRespawn(RenderableEntity* entity, EntityCombatState& state, uint64_t now);
        void UpdateBarrierState(RenderableEntity* entity, EntityCombatState& state, uint64_t now);
        void ProcessHealthChanges(RenderableEntity* entity, EntityCombatState& state, uint64_t now);
        void TriggerDamageFlushIfNeeded(EntityCombatState& state, uint64_t now);
        void UpdatePositionHistory(EntityCombatState& state, const RenderableEntity* entity, uint64_t now);
	};
} // namespace kx
