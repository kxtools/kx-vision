#pragma once
#include <cstdint>

namespace kx {
    // Forward declarations
    struct EntityCombatState;
    struct GameEntity;

    /**
     * @brief Pure logic container for combat state transitions.
     * 
     * This class is stateless. It takes an entity and its mutable state,
     * applies game rules (damage detection, respawn checks, animations),
     * and updates the state object.
     */
    class CombatLogic {
    public:
        /**
         * @brief Main entry point to update a single entity's state for this frame.
         * @param state The mutable combat state to update.
         * @param entity The read-only entity data from the current frame.
         * @param now Current timestamp in milliseconds.
         */
        static void UpdateState(EntityCombatState& state, const GameEntity* entity, uint64_t now);

    private:
        // Internal logic helpers
        static void UpdateDamageAccumulatorAnimation(EntityCombatState& state, uint64_t now);
        static bool HandleAttributeChanges(const GameEntity* entity, EntityCombatState& state, uint64_t now);
        static void ResetForRespawn(EntityCombatState& state, float currentHealth, uint64_t now);
        
        static void UpdateBarrierState(const GameEntity* entity, EntityCombatState& state, uint64_t now);
        
        static void ProcessHealthChanges(const GameEntity* entity, EntityCombatState& state, uint64_t now);
        static void HandleDamage(EntityCombatState& state, float currentHealth, uint64_t now);
        static void HandleHealing(EntityCombatState& state, float currentHealth, uint64_t now);
        
        static void TriggerDamageFlushIfNeeded(EntityCombatState& state, uint64_t now);
        static void UpdatePositionHistory(EntityCombatState& state, const GameEntity* entity, uint64_t now);
    };
}

