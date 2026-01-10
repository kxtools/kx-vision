#pragma once

#include "GameEntity.h"

namespace kx {

struct NpcEntity : public GameEntity {
    char name[64] = { 0 };
    uint32_t level;
    Game::Attitude attitude;
    Game::CharacterRank rank;

    NpcEntity() : GameEntity(),
                      level(0), attitude(Game::Attitude::Neutral), rank()
    {
        entityType = EntityTypes::NPC;
    }
};

} // namespace kx

