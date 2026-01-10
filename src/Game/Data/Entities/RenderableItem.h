#pragma once

#include "RenderableEntity.h"

namespace kx {

struct RenderableItem : public RenderableEntity {
    Game::ItemRarity rarity = Game::ItemRarity::None;
    uint32_t itemId = 0;

    RenderableItem() : RenderableEntity() {
        entityType = EntityTypes::Item;
    }
};

} // namespace kx

