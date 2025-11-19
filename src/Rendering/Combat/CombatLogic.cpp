#include "CombatLogic.h"
#include "CombatState.h"
#include "../Data/RenderableData.h"
#include "../../Core/AppState.h"
#include <glm/geometric.hpp>

#include "Shared/CombatConstants.h"

namespace kx {

    namespace {
        constexpr float MIN_POSITION_CHANGE = 0.1f;
    }

    void CombatLogic::UpdateState(EntityCombatState& state, const RenderableEntity* entity, uint64_t now)
    {
        const float currentHealth = entity->currentHealth;
        const float currentMaxHealth = entity->maxHealth;

        // 1. Animate fade-outs for damage bars
        UpdateDamageAccumulatorAnimation(state, now);
        
        // 2. Check for Respawns/Mounting/Phase Changes
        // If this returns true, the state was reset, so we stop processing changes this frame.
        if (DetectStateChangeOrRespawn(entity, state, now)) {
            return; 
        }

        // 3. Update Barrier
        UpdateBarrierState(entity, state, now);

        // 4. Detect Damage/Healing
        ProcessHealthChanges(entity, state, now);

        // 5. Check if damage numbers should pop up (Accumulation logic)
        TriggerDamageFlushIfNeeded(state, now);
        
        // 6. Update movement history (Trails)
        UpdatePositionHistory(state, entity, now);
        
        // --- Final State Update for Next Frame ---
        state.lastKnownHealth = currentHealth;
        state.lastKnownMaxHealth = currentMaxHealth;
        state.lastKnownBarrier = entity->currentBarrier;
        state.lastSeenTimestamp = now;
    }

    void CombatLogic::UpdateDamageAccumulatorAnimation(EntityCombatState& state, uint64_t now)
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

    bool CombatLogic::DetectStateChangeOrRespawn(const RenderableEntity* entity, EntityCombatState& state, uint64_t now)
    {
        const float currentHealth = entity->currentHealth;
        const float currentMaxHealth = entity->maxHealth;

        // Case 1: Max health changes (handles downed state, mounting, phase transitions, address reuse)
        if (state.lastKnownMaxHealth > 0 && abs(currentMaxHealth - state.lastKnownMaxHealth) > 1.0f)
        {
            ResetForRespawn(state, currentHealth, now);
            state.lastKnownMaxHealth = currentMaxHealth; // Update max health after reset
            return true;
        }

        // Case 2: Instant destruction from full health (gadget-only behavior)
        if (entity->entityType == EntityTypes::Gadget)
        {
            if (state.lastKnownMaxHealth > 0 &&
                state.lastKnownHealth >= state.lastKnownMaxHealth && // Was at full health
                currentHealth <= 0.0f) // Is now at zero or less
            {
                ResetForRespawn(state, currentHealth, now);
                return true;
            }
        }

        return false;
    }

    void CombatLogic::ResetForRespawn(EntityCombatState& state, float currentHealth, uint64_t now)
    {
        // Preserve current barrier state to prevent phantom barrier change detection
        const float currentBarrier = state.lastKnownBarrier;
        
        state = {};
        state.lastKnownHealth = currentHealth;
        state.lastKnownBarrier = currentBarrier; // Preserve barrier state
        state.lastSeenTimestamp = now;
        // No heal effects triggered; respawn is treated as a fresh baseline.
    }

    void CombatLogic::UpdateBarrierState(const RenderableEntity* entity, EntityCombatState& state, uint64_t now)
    {
        const float currentBarrier = entity->currentBarrier;
        if (currentBarrier != state.lastKnownBarrier)
        {
            state.barrierOnLastChange = state.lastKnownBarrier;
            state.lastBarrierChangeTimestamp = now;
        }
    }

    void CombatLogic::ProcessHealthChanges(const RenderableEntity* entity, EntityCombatState& state, uint64_t now)
    {
        const float currentHealth = entity->currentHealth;
        
        // Only process changes if we have seen this entity before (prevent diffing against 0 on first frame)
        if (state.lastSeenTimestamp > 0)
        {
            if (currentHealth < state.lastKnownHealth)
            {
                HandleDamage(state, currentHealth, now);
            }
            else if (currentHealth > state.lastKnownHealth)
            {
                HandleHealing(state, currentHealth, now);
            }
        }
    }

    void CombatLogic::HandleDamage(EntityCombatState& state, float currentHealth, uint64_t now)
    {
        const float damage = state.lastKnownHealth - currentHealth;
        if (damage <= 0.0f) return;

        // If this is the first damage in a new burst, record the start time.
        if (state.accumulatedDamage <= 0.0f)
        {
            state.burstStartTime = now;
        }

        state.accumulatedDamage += damage;
    
        state.lastDamageTaken = damage;
        state.lastHitTimestamp = now;
    
        if (currentHealth <= 0.0f && state.deathTimestamp == 0)
        {
            state.deathTimestamp = now;
        }
    }

    void CombatLogic::HandleHealing(EntityCombatState& state, float currentHealth, uint64_t now)
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
    }

    void CombatLogic::TriggerDamageFlushIfNeeded(EntityCombatState& state, uint64_t now)
    {
        if (state.flushAnimationStartTime == 0 && state.accumulatedDamage > 0.0f)
        {
            bool shouldFlush = false;

            // PRIORITY 1: Death Trigger. 
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

    void CombatLogic::UpdatePositionHistory(EntityCombatState& state, const RenderableEntity* entity, uint64_t now)
    {
        // Access settings singleton safely
        const auto& settings = AppState::Get().GetSettings();
        const size_t MAX_HISTORY_POINTS = static_cast<size_t>(settings.playerESP.trails.maxPoints);
        
        bool shouldRecordPosition = state.positionHistory.empty();
        if (!shouldRecordPosition) {
            const glm::vec3& lastPos = state.positionHistory.back().position;
            float distanceMoved = glm::distance(entity->position, lastPos);
            shouldRecordPosition = (distanceMoved >= MIN_POSITION_CHANGE);
        }
        
        if (shouldRecordPosition) {
            PositionHistoryPoint newPoint;
            newPoint.position = entity->position;
            newPoint.timestamp = now;
            state.positionHistory.push_back(newPoint);
            
            if (state.positionHistory.size() > MAX_HISTORY_POINTS) {
                state.positionHistory.pop_front();
            }
        }
    }

} // namespace kx

