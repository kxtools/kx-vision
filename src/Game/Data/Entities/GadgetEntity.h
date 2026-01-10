#pragma once

#include "GameEntity.h"

namespace kx {

struct GadgetEntity : public GameEntity {
    char name[64] = { 0 };
    Game::GadgetType type;
    Game::ResourceNodeType resourceType;
    bool isGatherable;
    
    GadgetEntity() : GameEntity(),
                         type(Game::GadgetType::None), resourceType(),
                         isGatherable(false)
    {
        entityType = EntityTypes::Gadget;
    }
};

} // namespace kx

