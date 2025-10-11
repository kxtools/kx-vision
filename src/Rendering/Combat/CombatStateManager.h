#pragma once

#include <unordered_map>
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
		 * @brief Post-update step for layout-dependent calculations (e.g., damage chunking).
		 * @param entity The entity to process.
		 * @param barWidth The final calculated width of the health bar for this entity.
		 */
		void PostUpdate(const RenderableEntity* entity, float barWidth);

		/**
		 * @brief Remove stale entries that have not been seen recently.
		 * Uses GetTickCount64 internally. If you want determinism/testability,
		 * a variant taking 'now' could be added.
		 */
		void Cleanup();

		/**
		 * @brief Get immutable pointer to stored entity combat state (nullptr if missing).
		 */
		const EntityCombatState* GetState(const void* entityId) const;
		EntityCombatState* GetStateNonConst(const void* entityId); // New method to get non-const state

	private:
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
	};
} // namespace kx
