#pragma once

#include "RenderableEntity.h"
#include <string>

namespace kx {

struct RenderableNpc : public RenderableEntity {
    std::string name;
    uint32_t level;
    Game::Attitude attitude;
    Game::CharacterRank rank;

    RenderableNpc() : RenderableEntity(),
                      level(0), attitude(Game::Attitude::Neutral), rank()
    {
        entityType = EntityTypes::NPC;
    }
};

} // namespace kx

