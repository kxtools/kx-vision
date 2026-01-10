#pragma once

#include "RenderableEntity.h"

namespace kx {

struct RenderableGadget : public RenderableEntity {
    char name[64] = { 0 };
    Game::GadgetType type;
    Game::ResourceNodeType resourceType;
    bool isGatherable;
    
    RenderableGadget() : RenderableEntity(),
                         type(Game::GadgetType::None), resourceType(),
                         isGatherable(false)
    {
        entityType = EntityTypes::Gadget;
    }
};

} // namespace kx

