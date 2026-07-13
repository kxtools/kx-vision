#pragma once

#include <cstdint>

// Forward declarations to avoid including heavy headers
namespace kx {
    struct GameEntity;
    struct CombatObservation;
    struct VisualCombatState;
    struct HealthBarAnimationState;
}

namespace kx {
    void PopulateHealthBarAnimations(
        const GameEntity* entity,
        const CombatObservation* observation,
        const VisualCombatState* visualState,
        HealthBarAnimationState& animState,
        uint64_t now);
} // namespace kx
