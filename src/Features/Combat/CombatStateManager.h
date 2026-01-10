#pragma once

#include <ankerl/unordered_dense.h>
#include <vector>
#include "CombatState.h"
#include "CombatStateKey.h"
#include "../../Game/Data/RenderableData.h" // For RenderableEntity

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
		void Update(const std::vector<GameEntity*>& entities, uint64_t now);

		/**
		 * @brief Remove combat state for entities that are no longer present in the game.
		 * @param activeKeys Set of entity keys that are currently active in the game.
		 */
		void Prune(const ankerl::unordered_dense::set<CombatStateKey, CombatStateKeyHash>& activeKeys);

		/**
		 * @brief Get immutable pointer to stored entity combat state (nullptr if missing).
		 */
		const EntityCombatState* GetState(CombatStateKey key) const;

	private:
		EntityCombatState* GetStateNonConst(CombatStateKey key);
		ankerl::unordered_dense::map<CombatStateKey, EntityCombatState, CombatStateKeyHash> m_entityStates;

		EntityCombatState& AcquireState(CombatStateKey key);
	};
} // namespace kx
