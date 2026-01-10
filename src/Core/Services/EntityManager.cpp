#include "EntityManager.h"
#include "../../Game/Extraction/DataExtractor.h"
#include "../AppState.h"
#include <algorithm>

namespace kx {

EntityManager::EntityManager() = default;

void EntityManager::Update(uint64_t now) {
    const float currentTimeSeconds = now / 1000.0f;
    const Settings& settings = AppState::Get().GetSettings();
    const float updateInterval = 1.0f / (std::max)(1.0f, settings.espUpdateRate);

    if (currentTimeSeconds - m_lastGameDataUpdateTime >= updateInterval) {
        // Reset all object pools
        m_playerPool.Reset();
        m_npcPool.Reset();
        m_gadgetPool.Reset();
        m_attackTargetPool.Reset();
        m_itemPool.Reset();
        m_frameData.Reset();

        // Extract entity data from game memory
        DataExtractor::ExtractFrameData(
            m_playerPool, m_npcPool, m_gadgetPool, 
            m_attackTargetPool, m_itemPool, m_frameData
        );

        // Collect all combat state keys
        const size_t totalCount = m_frameData.players.size() + m_frameData.npcs.size() +
            m_frameData.gadgets.size() + m_frameData.attackTargets.size() +
            m_frameData.items.size();

        m_activeCombatKeys.clear();
        m_activeCombatKeys.reserve(totalCount);

        auto collectKeys = [&](const auto& collection) {
            for (const auto* e : collection) {
                m_activeCombatKeys.insert(e->GetCombatKey());
            }
        };

        collectKeys(m_frameData.players);
        collectKeys(m_frameData.npcs);
        collectKeys(m_frameData.gadgets);
        collectKeys(m_frameData.attackTargets);
        collectKeys(m_frameData.items);

        // Prune stale combat states
        m_combatStateManager.Prune(m_activeCombatKeys);

        // Update combat states
        m_allEntitiesBuffer.clear();
        if (m_allEntitiesBuffer.capacity() < totalCount) {
            m_allEntitiesBuffer.reserve(totalCount);
        }

        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameData.players.begin(), m_frameData.players.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameData.npcs.begin(), m_frameData.npcs.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameData.gadgets.begin(), m_frameData.gadgets.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameData.attackTargets.begin(), m_frameData.attackTargets.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameData.items.begin(), m_frameData.items.end());
        m_combatStateManager.Update(m_allEntitiesBuffer, now);

        // Update adaptive far plane
        AppState::Get().UpdateAdaptiveFarPlane(m_frameData);

        m_lastGameDataUpdateTime = currentTimeSeconds;
    }
}

void EntityManager::Reset() {
    m_playerPool.Reset();
    m_npcPool.Reset();
    m_gadgetPool.Reset();
    m_attackTargetPool.Reset();
    m_itemPool.Reset();
    m_frameData.Reset();
    m_activeCombatKeys.clear();
    m_allEntitiesBuffer.clear();
}

} // namespace kx
