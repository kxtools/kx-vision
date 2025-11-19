#pragma once

#include "RenderableEntity.h"
#include <string>

namespace kx {

struct RenderableGadget : public RenderableEntity {
    std::string name;
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

