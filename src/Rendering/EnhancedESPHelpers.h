#pragma once

#include "../Game/GameEnums.h"
#include "StringHelpers.h"
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
    // Convert attitude enum to display string
    static std::string AttitudeToString(Game::Attitude attitude) {
        const char* name = Game::EnumHelpers::GetAttitudeName(attitude);
        if (name != nullptr) {
            return std::string(name);
        }
        return "Attitude ID: " + std::to_string(static_cast<uint32_t>(attitude));
    }

    // Convert gadget type enum to display string
    static std::string GadgetTypeToString(Game::GadgetType type) {
        const char* name = Game::EnumHelpers::GetGadgetTypeName(type);
        if (name != nullptr) {
            return std::string(name);
        }
        return "Gadget ID: " + std::to_string(static_cast<uint32_t>(type));
    }

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

    // Check if a profession is a heavy armor class
    static bool IsHeavyArmorProfession(Game::Profession profession) {
        return profession == Game::Profession::Guardian ||
               profession == Game::Profession::Warrior;
    }

    // Check if a profession is a medium armor class
    static bool IsMediumArmorProfession(Game::Profession profession) {
        return profession == Game::Profession::Engineer ||
               profession == Game::Profession::Ranger ||
               profession == Game::Profession::Thief;
    }

    // Check if a profession is a light armor class
    static bool IsLightArmorProfession(Game::Profession profession) {
        return profession == Game::Profession::Elementalist ||
               profession == Game::Profession::Mesmer ||
               profession == Game::Profession::Necromancer ||
               profession == Game::Profession::Revenant;
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
