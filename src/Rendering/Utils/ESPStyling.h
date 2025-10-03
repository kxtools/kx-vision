#pragma once

#include "../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"

// Forward declaration to avoid circular dependency
namespace kx {
namespace Filtering {
    class EntityFilter;
}
}

namespace kx {

// Enhanced helper functions using the new enums
class ESPHelpers {
public:

    static ImU32 GetRarityColor(Game::ItemRarity rarity) {
        switch (rarity) {
            case Game::ItemRarity::Junk:       return IM_COL32(170, 170, 170, 255); // Gray (#AAAAAA)
            case Game::ItemRarity::Common:     return IM_COL32(255, 255, 255, 255); // White (readability over authenticity)
            case Game::ItemRarity::Fine:       return IM_COL32(98, 164, 218, 255);  // Blue (#62A4DA)
            case Game::ItemRarity::Masterwork: return IM_COL32(26, 147, 6, 255);    // Green (#1a9306)
            case Game::ItemRarity::Rare:       return IM_COL32(252, 208, 11, 255);  // Yellow (#fcd00b)
            case Game::ItemRarity::Exotic:     return IM_COL32(255, 164, 5, 255);   // Orange (#ffa405)
            case Game::ItemRarity::Ascended:   return IM_COL32(251, 62, 141, 255);  // Pink (#fb3e8d)
            case Game::ItemRarity::Legendary:  return IM_COL32(139, 79, 219, 255);  // Bright Purple (#8B4FDB - brightened for readability)
            default:                           return IM_COL32(255, 255, 255, 255); // Default to white
        }
    }

    // Get color based on gadget type
    static unsigned int GetGadgetTypeColor(Game::GadgetType type) {
        switch (type) {
            case Game::GadgetType::ResourceNode:
                return 0xDC32FF32; // Bright Green
            case Game::GadgetType::Waypoint:
                return 0xDC32FFFF; // Cyan
            case Game::GadgetType::Vista:
                return 0xDCFF32FF; // Magenta
            case Game::GadgetType::Crafting:
                return 0xDCFF8032; // Orange
            case Game::GadgetType::AttackTarget:
                return 0xDCFF3232; // Red
            case Game::GadgetType::PlayerCreated:
                return 0xDC8032FF; // Purple
            case Game::GadgetType::Interact:
                return 0xDCFFFF32; // Yellow
            case Game::GadgetType::Door:
                return 0xDC808080; // Gray
            default:
                return 0xDCC8C8C8; // Light Gray
        }
    }


};

} // namespace kx
