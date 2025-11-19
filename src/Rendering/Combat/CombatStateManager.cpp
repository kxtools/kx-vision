#include "CombatStateManager.h"

#include "CombatLogic.h"

namespace kx
{
	EntityCombatState& CombatStateManager::AcquireState(CombatStateKey key)
	{
		return m_entityStates[key];
	}

	void CombatStateManager::Update(const std::vector<RenderableEntity*>& entities, uint64_t now)
	{
		for (auto* entity : entities)
		{
			if (!entity || !entity->isValid || entity->maxHealth <= 0.0f)
			{
				continue;
			}

			CombatStateKey key = entity->GetCombatKey();
			EntityCombatState& state = AcquireState(key);
			CombatLogic::UpdateState(state, entity, now);
		}
	}

	const EntityCombatState* CombatStateManager::GetState(CombatStateKey key) const
	{
		auto it = m_entityStates.find(key);
		return (it != m_entityStates.end()) ? &it->second : nullptr;
	}

	EntityCombatState* CombatStateManager::GetStateNonConst(CombatStateKey key)
	{
		auto it = m_entityStates.find(key);
		return (it != m_entityStates.end()) ? &it->second : nullptr;
	}

	void CombatStateManager::Prune(const ankerl::unordered_dense::set<CombatStateKey>& activeKeys)
	{
		for (auto it = m_entityStates.begin(); it != m_entityStates.end();)
		{
			if (activeKeys.find(it->first) == activeKeys.end()) {
				it = m_entityStates.erase(it);
			} else {
				++it;
			}
		}
	}
} // namespace kx
