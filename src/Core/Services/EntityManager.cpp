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
        m_frameDataWorkBuffer.Reset();

        // Extract entity data from game memory into work buffer
        DataExtractor::ExtractFrameData(
            m_playerPool, m_npcPool, m_gadgetPool, 
            m_attackTargetPool, m_itemPool, m_frameDataWorkBuffer
        );

        // Collect all combat state keys from work buffer
        const size_t totalCount = m_frameDataWorkBuffer.players.size() + m_frameDataWorkBuffer.npcs.size() +
            m_frameDataWorkBuffer.gadgets.size() + m_frameDataWorkBuffer.attackTargets.size() +
            m_frameDataWorkBuffer.items.size();

        m_activeCombatKeys.clear();
        m_activeCombatKeys.reserve(totalCount);

        auto collectKeys = [&](const auto& collection) {
            for (const auto* e : collection) {
                m_activeCombatKeys.insert(e->GetCombatKey());
            }
        };

        collectKeys(m_frameDataWorkBuffer.players);
        collectKeys(m_frameDataWorkBuffer.npcs);
        collectKeys(m_frameDataWorkBuffer.gadgets);
        collectKeys(m_frameDataWorkBuffer.attackTargets);
        collectKeys(m_frameDataWorkBuffer.items);

        // Prune stale combat states
        m_combatStateManager.Prune(m_activeCombatKeys);

        // Update combat states
        m_allEntitiesBuffer.clear();
        if (m_allEntitiesBuffer.capacity() < totalCount) {
            m_allEntitiesBuffer.reserve(totalCount);
        }

        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameDataWorkBuffer.players.begin(), m_frameDataWorkBuffer.players.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameDataWorkBuffer.npcs.begin(), m_frameDataWorkBuffer.npcs.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameDataWorkBuffer.gadgets.begin(), m_frameDataWorkBuffer.gadgets.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameDataWorkBuffer.attackTargets.begin(), m_frameDataWorkBuffer.attackTargets.end());
        m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), m_frameDataWorkBuffer.items.begin(), m_frameDataWorkBuffer.items.end());
        m_combatStateManager.Update(m_allEntitiesBuffer, now);

        // Update adaptive far plane
        AppState::Get().UpdateAdaptiveFarPlane(m_frameDataWorkBuffer);

        // Atomically swap work buffer to snapshot for thread-safe Render Thread access
        {
            std::lock_guard<std::mutex> lock(m_snapshotMutex);
            m_frameDataSnapshot = m_frameDataWorkBuffer;
        }

        m_lastGameDataUpdateTime = currentTimeSeconds;
    }
}

void EntityManager::Reset() {
    m_playerPool.Reset();
    m_npcPool.Reset();
    m_gadgetPool.Reset();
    m_attackTargetPool.Reset();
    m_itemPool.Reset();
    m_frameDataWorkBuffer.Reset();
    {
        std::lock_guard<std::mutex> lock(m_snapshotMutex);
        m_frameDataSnapshot.Reset();
    }
    m_activeCombatKeys.clear();
    m_allEntitiesBuffer.clear();
}

} // namespace kx
