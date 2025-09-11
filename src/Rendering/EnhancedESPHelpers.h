#pragma once

#include "../Game/GameEnums.h"
#include <string>

namespace kx {

// Enhanced helper functions using the new enums
class ESPHelpers {
public:
    // Convert profession enum to display string
    static std::string ProfessionToString(Game::Profession profession) {
        return std::string(Game::EnumHelpers::GetProfessionName(profession));
    }

    // Convert race enum to display string  
    static std::string RaceToString(Game::Race race) {
        return std::string(Game::EnumHelpers::GetRaceName(race));
    }

    // Convert attitude enum to display string
    static std::string AttitudeToString(Game::Attitude attitude) {
        return std::string(Game::EnumHelpers::GetAttitudeName(attitude));
    }

    // Convert gadget type enum to display string
    static std::string GadgetTypeToString(Game::GadgetType type) {
        return std::string(Game::EnumHelpers::GetGadgetTypeName(type));
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

    // Check if a gadget type should be considered important
    static bool IsImportantGadgetType(Game::GadgetType type) {
        switch (type) {
            case Game::GadgetType::ResourceNode:
            case Game::GadgetType::Waypoint:
            case Game::GadgetType::Vista:
            case Game::GadgetType::AttackTarget:
            case Game::GadgetType::Interact:
                return true;
            default:
                return false;
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
        if (IsHeavyArmorProfession(profession)) return "Heavy";
        if (IsMediumArmorProfession(profession)) return "Medium";
        if (IsLightArmorProfession(profession)) return "Light";
        return "Unknown";
    }
};

} // namespace kx
