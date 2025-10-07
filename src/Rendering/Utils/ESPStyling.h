#pragma once

#include "../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"
#include "ESPConstants.h"
#include "Generated/EnumsAndStructs.h" // <-- ADD THIS INCLUDE

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

    // Get color based on tactical role
    inline ImU32 GetTacticalColor(kx::data::ApiAttribute attribute) {
        switch (attribute) {
            // Offensive Stats -> Red
            case kx::data::ApiAttribute::Power:
            case kx::data::ApiAttribute::Precision:
            case kx::data::ApiAttribute::CritDamage:
            case kx::data::ApiAttribute::ConditionDamage:
                return IM_COL32(255, 80, 80, 255); // Red

            // Defensive Stats -> Blue
            case kx::data::ApiAttribute::Toughness:
            case kx::data::ApiAttribute::Vitality:
                return IM_COL32(30, 144, 255, 255); // Blue

            // Support Stats -> Green
            case kx::data::ApiAttribute::Healing:
            case kx::data::ApiAttribute::BoonDuration:
            case kx::data::ApiAttribute::ConditionDuration:
                return IM_COL32(100, 255, 100, 255); // Green

            default:
                return ESPColors::DEFAULT_TEXT;
        }
    }

} // namespace ESPStyling

} // namespace kx
