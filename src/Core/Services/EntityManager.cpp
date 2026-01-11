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
        // 1. Determine which pools to write to (back buffers)
        // We compute the next index but don't flip the global index yet
        const size_t nextIndex = (m_writeIndex + 1) % BUFFER_COUNT;

        // Get references to the back buffer pools
        auto& nextPlayerPool = m_playerPools[nextIndex];
        auto& nextNpcPool = m_npcPools[nextIndex];
        auto& nextGadgetPool = m_gadgetPools[nextIndex];
        auto& nextAttackTargetPool = m_attackTargetPools[nextIndex];
        auto& nextItemPool = m_itemPools[nextIndex];

        // 2. Reset ONLY the back buffer pools and scratch buffers
        nextPlayerPool.Reset();
        nextNpcPool.Reset();
        nextGadgetPool.Reset();
        nextAttackTargetPool.Reset();
        nextItemPool.Reset();
        m_frameDataWorkBuffer.Reset();

        // Clear the persistent character-to-name map (retains capacity for high-water mark strategy)
        m_charToNameMap.clear();

        // 3. Extract entity data from game memory into work buffer and back buffer pools
        const bool extracted = DataExtractor::ExtractFrameData(
            nextPlayerPool, nextNpcPool, nextGadgetPool,
            nextAttackTargetPool, nextItemPool, m_frameDataWorkBuffer,
            m_charToNameMap
        );

        if (extracted) {
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

            // 4. Atomically publish: Swap work buffer to snapshot AND flip the write index
            // Both operations happen inside the lock to ensure render thread sees consistent state
            {
                std::lock_guard<std::mutex> lock(m_snapshotMutex);
                m_frameDataSnapshot = m_frameDataWorkBuffer;
                
                // CRITICAL: Flip the write index inside the lock.
                // This ensures the render thread (which grabbed the snapshot) is now 
                // officially "using" the data from the new pool index.
                // The NEXT update will target the old index (which is now safe to overwrite).
                m_writeIndex = nextIndex;
            }
        }

        m_lastGameDataUpdateTime = currentTimeSeconds;
    }
}

void EntityManager::Reset() {
    for (size_t i = 0; i < BUFFER_COUNT; ++i) {
        m_playerPools[i].Reset();
        m_npcPools[i].Reset();
        m_gadgetPools[i].Reset();
        m_attackTargetPools[i].Reset();
        m_itemPools[i].Reset();
    }
    m_frameDataWorkBuffer.Reset();
    {
        std::lock_guard<std::mutex> lock(m_snapshotMutex);
        m_frameDataSnapshot.Reset();
    }
    m_activeCombatKeys.clear();
    m_allEntitiesBuffer.clear();
    m_writeIndex = 0;
}

} // namespace kx
