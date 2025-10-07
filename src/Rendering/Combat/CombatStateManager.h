#pragma once

#include <unordered_map>
#include <vector>
#include "CombatState.h"
#include "../Data/RenderableData.h" // For RenderableEntity

namespace kx {
    class CombatStateManager {
    public:
        void Update(const std::vector<RenderableEntity*>& entities);
        void Cleanup();
        const EntityCombatState* GetState(const void* entityId) const;

    private:
        std::unordered_map<const void*, EntityCombatState> m_entityStates;
    };
}
