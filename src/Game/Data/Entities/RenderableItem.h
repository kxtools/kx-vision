#pragma once

#include "GameEntity.h"

namespace kx {

struct RenderableItem : public GameEntity {
    Game::ItemRarity rarity = Game::ItemRarity::None;
    uint32_t itemId = 0;

    RenderableItem() : GameEntity() {
        entityType = EntityTypes::Item;
    }
};

} // namespace kx

