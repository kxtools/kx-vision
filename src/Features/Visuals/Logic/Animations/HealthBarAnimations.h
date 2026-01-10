#pragma once

#include <cstdint>

// Forward declarations to avoid including heavy headers
namespace kx {
    struct GameEntity;
    struct EntityCombatState;
    struct HealthBarAnimationState;
}

namespace kx {
    void PopulateHealthBarAnimations(const GameEntity* entity, const EntityCombatState* state, HealthBarAnimationState& animState, uint64_t now);
} // namespace kx
