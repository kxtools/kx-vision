#pragma once

#include "../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"
#include "ESPConstants.h"

// Forward declaration to avoid circular dependency
namespace kx {
namespace Filtering {
    class EntityFilter;
}
}

namespace kx {

namespace ESPStyling {

    inline ImU32 GetRarityColor(Game::ItemRarity rarity) {
        switch (rarity) {
            case Game::ItemRarity::Junk:       return kx::RarityColors::JUNK;
            case Game::ItemRarity::Common:     return kx::RarityColors::COMMON;
            case Game::ItemRarity::Fine:       return kx::RarityColors::FINE;
            case Game::ItemRarity::Masterwork: return kx::RarityColors::MASTERWORK;
            case Game::ItemRarity::Rare:       return kx::RarityColors::RARE;
            case Game::ItemRarity::Exotic:     return kx::RarityColors::EXOTIC;
            case Game::ItemRarity::Ascended:   return kx::RarityColors::ASCENDED;
            case Game::ItemRarity::Legendary:  return kx::RarityColors::LEGENDARY;
            default:                           return kx::RarityColors::DEFAULT;
        }
    }

    // You can add more mapping functions here in the future, e.g.:
    // inline const char* GetIconForProfession(Game::Profession prof) { ... }

} // namespace ESPStyling

} // namespace kx
