#pragma once

#include "RenderableEntity.h"

namespace kx {

struct RenderableAttackTarget : public RenderableEntity {
    Game::AttackTargetCombatState combatState = Game::AttackTargetCombatState::Idle;
    
    RenderableAttackTarget() : RenderableEntity()
    {
        entityType = EntityTypes::AttackTarget;
    }
};

} // namespace kx

