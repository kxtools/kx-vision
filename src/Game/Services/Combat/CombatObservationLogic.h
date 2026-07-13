#pragma once

#include <cstdint>

namespace kx {

struct CombatObservation;
struct GameEntity;

// Stateless game-thread transition detector. Produces facts; owns no presentation policy.
class CombatObservationLogic {
public:
    static void Update(CombatObservation& observation, const GameEntity& entity, uint64_t now);

private:
    static bool HandleAttributeChanges(const GameEntity& entity, CombatObservation& observation, uint64_t now);
    static void ResetForRespawn(CombatObservation& observation, float currentHealth, uint64_t now);
    static void UpdateBarrier(const GameEntity& entity, CombatObservation& observation, uint64_t now);
    static void ProcessHealthChanges(const GameEntity& entity, CombatObservation& observation, uint64_t now);
};

} // namespace kx
