#include "CombatStateManager.h"
#include "../Utils/ESPConstants.h"
#include <Windows.h>
#include <algorithm>

namespace kx {
    void CombatStateManager::Update(const std::vector<RenderableEntity*>& entities) {
    uint64_t now = GetTickCount64();

    for (const auto* entity : entities) {
        if (!entity || !entity->isValid) continue;

        // Only track entities with health
        if (entity->maxHealth <= 0.0f) continue;

        const void* entityId = entity->address;
        float currentHealth = entity->currentHealth;

        auto& state = m_entityStates[entityId];

        if (state.lastSeenTimestamp > 0) {
            // --- NEW: DEATH DETECTION LOGIC ---
            // Check if the entity just died THIS frame.
            if (currentHealth <= 0.0f && state.lastKnownHealth > 0.0f) {
                state.deathTimestamp = now;
            }
            // If the entity is alive, make sure the death timestamp is cleared
            // (in case they resurrect).
            else if (currentHealth > 0.0f) {
                state.deathTimestamp = 0;
            }

            if (currentHealth < state.lastKnownHealth) {
                // --- Damage Logic ---
                state.lastDamageTaken = state.lastKnownHealth - currentHealth;
                state.lastHitTimestamp = now;
            }
            else if (currentHealth > state.lastKnownHealth) {
                // --- FINAL, PERFECTED HEALING LOGIC ---
                // If the last heal was a little while ago, this starts a NEW burst event.
                if (now - state.lastHealTimestamp > CombatEffects::BURST_HEAL_WINDOW_MS) {
                    // This is a new heal. Reset the "floor" of the overlay.
                    state.healStartHealth = state.lastKnownHealth;
                }
                
                // Always update the timestamps to keep the effects active during the heal.
                state.lastHealTimestamp = now;
                state.lastHealFlashTimestamp = now;
            }
        }
        
        // Update the state for the next frame
        state.lastKnownHealth = currentHealth;
        state.lastSeenTimestamp = now;
    }
}

    const EntityCombatState* CombatStateManager::GetState(const void* entityId) const {
        auto it = m_entityStates.find(entityId);
        if (it != m_entityStates.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void CombatStateManager::Cleanup() {
        uint64_t now = GetTickCount64();

        for (auto it = m_entityStates.begin(); it != m_entityStates.end(); ) {
            if (now - it->second.lastSeenTimestamp > CombatEffects::STATE_CLEANUP_THRESHOLD_MS) {
                it = m_entityStates.erase(it);
            } else {
                ++it;
            }
        }
    }
}
