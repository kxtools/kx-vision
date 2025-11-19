#include "CombatStateManager.h"

#include "CombatLogic.h"

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

			// 1. Get Storage
			EntityCombatState& state = AcquireState(entity);
			
			// 2. Delegate Logic
			CombatLogic::UpdateState(state, entity, now);
		}
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

	void CombatStateManager::Prune(const std::unordered_set<const void*>& activeEntities)
	{
		for (auto it = m_entityStates.begin(); it != m_entityStates.end();)
		{
			// If the entity address is NOT in the active list, it's truly gone.
			if (activeEntities.find(it->first) == activeEntities.end()) {
				it = m_entityStates.erase(it);
			} else {
				++it;
			}
		}
	}
} // namespace kx
