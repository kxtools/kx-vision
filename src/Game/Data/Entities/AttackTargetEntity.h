#pragma once

#include "GameEntity.h"

namespace kx {

struct AttackTargetEntity : public GameEntity {
    Game::AttackTargetCombatState combatState = Game::AttackTargetCombatState::Idle;
    
    AttackTargetEntity() : GameEntity()
    {
        entityType = EntityTypes::AttackTarget;
    }
};

} // namespace kx

