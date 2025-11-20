#pragma once

#include "RenderableEntity.h"

namespace kx {

struct RenderableNpc : public RenderableEntity {
    char name[64] = { 0 };
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

