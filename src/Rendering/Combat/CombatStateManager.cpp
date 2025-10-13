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

	void CombatStateManager::UpdateDamageAccumulatorAnimation(EntityCombatState& state, uint64_t now)
	{
		if (state.flushAnimationStartTime > 0)
		{
			const uint64_t elapsed = now - state.flushAnimationStartTime;
			if (elapsed >= CombatEffects::DAMAGE_ACCUMULATOR_FADE_MS)
			{
				// Animation is complete. Reset for the next damage burst.
				state.accumulatedDamage = 0.0f;
				state.flushAnimationStartTime = 0;
				state.damageToDisplay = 0.0f;
			}
		}
	}

	bool CombatStateManager::DetectStateChangeOrRespawn(RenderableEntity* entity, EntityCombatState& state, uint64_t now)
	{
		const float currentHealth = entity->currentHealth;
		const float currentMaxHealth = entity->maxHealth;

		// --- Gadget State Change Detection ---
		if (entity->entityType == ESPEntityType::Gadget && state.lastKnownMaxHealth > 0 && abs(currentMaxHealth - state.lastKnownMaxHealth) > 1.0f)
		{
			ResetForRespawn(state, currentHealth, now);
			state.lastKnownMaxHealth = currentMaxHealth; // Update max health after reset
			return true; // State was reset, skip further processing this frame
		}
		return false;
	}

	void CombatStateManager::UpdateBarrierState(RenderableEntity* entity, EntityCombatState& state, uint64_t now)
	{
		const float currentBarrier = entity->currentBarrier;
		if (currentBarrier != state.lastKnownBarrier)
		{
			state.barrierOnLastChange = state.lastKnownBarrier;
			state.lastBarrierChangeTimestamp = now;
		}
	}

	void CombatStateManager::ProcessHealthChanges(RenderableEntity* entity, EntityCombatState& state, uint64_t now)
	{
		const float currentHealth = entity->currentHealth;
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
	}

	void CombatStateManager::TriggerDamageFlushIfNeeded(EntityCombatState& state, uint64_t now)
	{
		if (state.flushAnimationStartTime == 0 && state.accumulatedDamage > 0.0f)
		{
			bool shouldFlush = false;

			// PRIORITY 1: Death Trigger. If the target is dead, use a very short timeout to flush the final damage number.
			if (state.deathTimestamp > 0)
			{
				if (now - state.lastHitTimestamp > CombatEffects::POST_MORTEM_FLUSH_DELAY_MS)
				{
					shouldFlush = true;
				}
			}
			// PRIORITY 2 & 3: Lull and Max Duration Triggers (for living targets).
			else 
			{
				// Primary Trigger: A lull in combat.
				if (now - state.lastHitTimestamp > CombatEffects::BURST_INACTIVITY_TIMEOUT_MS)
				{
					shouldFlush = true;
				}
				// Secondary Trigger: The burst has lasted for the maximum allowed duration.
				else if (state.burstStartTime > 0 && now - state.burstStartTime > CombatEffects::MAX_BURST_DURATION_MS)
				{
					shouldFlush = true;
				}
			}

			if (shouldFlush)
			{
				state.flushAnimationStartTime = now;
				state.damageToDisplay = state.accumulatedDamage;
			}
		}
	}

	void CombatStateManager::ProcessEntity(RenderableEntity* entity, uint64_t now)
	{
		EntityCombatState& state = AcquireState(entity);
		const float currentHealth = entity->currentHealth;
		const float currentMaxHealth = entity->maxHealth;

		UpdateDamageAccumulatorAnimation(state, now);
		
		if (DetectStateChangeOrRespawn(entity, state, now)) {
			return; // State was reset, skip further processing this frame
		}

		UpdateBarrierState(entity, state, now);

		ProcessHealthChanges(entity, state, now);

		TriggerDamageFlushIfNeeded(state, now);
		
		// --- Final State Update for Next Frame ---
		state.lastKnownHealth = currentHealth;
		state.lastKnownMaxHealth = currentMaxHealth;
		state.lastKnownBarrier = entity->currentBarrier;
		state.lastSeenTimestamp = now;
	}
	void CombatStateManager::HandleDamage(EntityCombatState& state,
			const RenderableEntity* entity,
			float currentHealth,
			uint64_t now)
	{
	    // Detect when a gadget instantly goes from full health to zero. This is a state change, not a death,
	    // so we reset its state without setting a death timestamp to prevent the death animation from playing incorrectly.
	    if (entity->entityType == ESPEntityType::Gadget &&
	        state.lastKnownMaxHealth > 0 &&
	        state.lastKnownHealth >= state.lastKnownMaxHealth && // Was at full health
	        currentHealth <= 0.0f) // Is now dead
	    {
	        ResetForRespawn(state, currentHealth, now);
	        return;
	    }
	
		const float damage = state.lastKnownHealth - currentHealth;
		if (damage <= 0.0f) return;

        // <<< ADD THIS BLOCK >>>
        // If this is the first damage in a new burst, record the start time.
        if (state.accumulatedDamage <= 0.0f)
        {
            state.burstStartTime = now;
        }
        // <<< END ADD >>>

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

	void CombatStateManager::Cleanup(uint64_t now)
	{
		// uint64_t now = GetTickCount64(); // This line is removed
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
