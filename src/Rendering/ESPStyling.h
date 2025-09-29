#pragma once

#include "../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"
#include <string>

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

    // Get color based on attitude
    static unsigned int GetAttitudeColor(Game::Attitude attitude) {
        switch (attitude) {
            case Game::Attitude::Friendly: 
                return 0xDC64FF00; // Green
            case Game::Attitude::Hostile: 
                return 0xDC3232FF; // Red
            case Game::Attitude::Indifferent: 
                return 0xDCFFFF32; // Yellow
            case Game::Attitude::Neutral: 
            default: 
                return 0xDCFFFFFF; // White
        }
    }

    static ImU32 GetRarityColor(Game::ItemRarity rarity) {
        switch (rarity) {
            case Game::ItemRarity::Junk:       return IM_COL32(100, 100, 100, 255); // Gray
            case Game::ItemRarity::Common:     return IM_COL32(189, 170, 170, 255); // White
            case Game::ItemRarity::Fine:       return IM_COL32(98, 164, 218, 255);  // Blue
            case Game::ItemRarity::Masterwork: return IM_COL32(26, 147, 6, 255);    // Green
            case Game::ItemRarity::Rare:       return IM_COL32(252, 208, 11, 255);  // Yellow
            case Game::ItemRarity::Exotic:     return IM_COL32(255, 164, 5, 255);   // Orange
            case Game::ItemRarity::Ascended:   return IM_COL32(251, 62, 141, 255);  // Pink
            case Game::ItemRarity::Legendary:  return IM_COL32(76, 19, 157, 255);   // Purple
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

    // Get armor weight description
    static std::string GetArmorWeight(Game::Profession profession) {
        const char* name = Game::EnumHelpers::GetArmorWeight(profession);
        if (name != nullptr) {
            return std::string(name);
        }
        return "Armor ID: " + std::to_string(static_cast<uint32_t>(profession));
    }
};

} // namespace kx
