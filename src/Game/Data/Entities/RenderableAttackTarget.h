#pragma once

#include "GameEntity.h"

namespace kx {

struct RenderableAttackTarget : public GameEntity {
    Game::AttackTargetCombatState combatState = Game::AttackTargetCombatState::Idle;
    
    RenderableAttackTarget() : GameEntity()
    {
        entityType = EntityTypes::AttackTarget;
    }
};

} // namespace kx

