#include "CombatStateManager.h"
#include "../Utils/ESPConstants.h"
#include <Windows.h>
#include <algorithm>

namespace kx
{
	EntityCombatState& CombatStateManager::AcquireState(const RenderableEntity* entity)
	{
		return m_entityStates[entity->address]; // creates if missing
	}

	void CombatStateManager::Update(const std::vector<RenderableEntity*>& entities, uint64_t now)
	{
		for (auto* entity : entities)
		{
			if (!entity || !entity->isValid || entity->maxHealth <= 0.0f)
			{
				continue;
			}
			ProcessEntity(entity, now);
		}
	}

	void CombatStateManager::ProcessEntity(RenderableEntity* entity, uint64_t now)
	{
		EntityCombatState& state = AcquireState(entity);
		const float currentHealth = entity->currentHealth;

		// This call is moved from here...
		// MaybeFlushAccumulator(state, entity, now); 

		if (state.lastSeenTimestamp > 0)
		{
			if (currentHealth < state.lastKnownHealth)
			{
				HandleDamage(state, entity, currentHealth, now);
			}
			else if (currentHealth > state.lastKnownHealth)
			{
				HandleHealing(state, entity, currentHealth, now);
			}
		}

		state.lastKnownHealth = currentHealth;
		state.lastSeenTimestamp = now;

		// The flush logic is now handled in the renderer.
	}

	void CombatStateManager::HandleDamage(EntityCombatState& state,
		const RenderableEntity* entity,
		float currentHealth,
		uint64_t now)
	{
		const float damage = state.lastKnownHealth - currentHealth;
		if (damage <= 0.0f) return;

		state.accumulatedDamage += damage;

		state.lastDamageTaken = damage;
		state.lastHitTimestamp = now;

		if (currentHealth <= 0.0f && state.deathTimestamp == 0)
		{
			state.deathTimestamp = now;
		}
	}

	void CombatStateManager::HandleHealing(EntityCombatState& state,
	                                       const RenderableEntity* entity,
	                                       float currentHealth,
	                                       uint64_t now)
	{
		// Respawn / resurrection detection: last known was zero or below.
		if (state.lastKnownHealth <= 0.0f)
		{
			ResetForRespawn(state, currentHealth, now);
			return;
		}

		// Genuine heal on a living entity
		if (now - state.lastHealTimestamp > CombatEffects::BURST_HEAL_WINDOW_MS)
		{
			state.healStartHealth = state.lastKnownHealth;
		}

		state.lastHealTimestamp = now;
		state.lastHealFlashTimestamp = now;

		// If entity was previously flagged dead but now > 0, ensure deathTimestamp stays (for fade) or reset?
		// Current behavior: we keep deathTimestamp until respawn detection resets it via ResetForRespawn().
		// Intentional: healing while dead isn't considered; only a health increase from 0 triggers respawn.
	}



	void CombatStateManager::ResetForRespawn(EntityCombatState& state,
	                                         float currentHealth,
	                                         uint64_t now)
	{
		// Reset everything; keep only what should logically persist if desired (currently nothing).
		state = {};
		state.lastKnownHealth = currentHealth;
		state.lastSeenTimestamp = now;
		// No heal effects triggered; respawn is treated as a fresh baseline.
	}

	const EntityCombatState* CombatStateManager::GetState(const void* entityId) const
	{
		auto it = m_entityStates.find(entityId);
		return (it != m_entityStates.end()) ? &it->second : nullptr;
	}

	EntityCombatState* CombatStateManager::GetStateNonConst(const void* entityId)
	{
		auto it = m_entityStates.find(entityId);
		return (it != m_entityStates.end()) ? &it->second : nullptr;
	}

	void CombatStateManager::Cleanup()
	{
		uint64_t now = GetTickCount64();
		for (auto it = m_entityStates.begin(); it != m_entityStates.end();)
		{
			if (now - it->second.lastSeenTimestamp > CombatEffects::STATE_CLEANUP_THRESHOLD_MS)
			{
				it = m_entityStates.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
} // namespace kx
