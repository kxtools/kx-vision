#pragma once

#include "GameEntity.h"

namespace kx {

struct ItemEntity : public GameEntity {
    Game::ItemRarity rarity = Game::ItemRarity::None;
    uint32_t itemId = 0;

    ItemEntity() : GameEntity() {
        entityType = EntityTypes::Item;
    }
};

} // namespace kx

