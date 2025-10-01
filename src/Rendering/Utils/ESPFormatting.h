#pragma once

#include <string>
#include "../../Game/GameEnums.h"

namespace kx {
namespace ESPFormatting {



// Legacy helpers for backward compatibility - these now use the new enums internally





// New helpers for the enhanced enum system







// Helper to format the character name with its rank prefix (e.g., "Veteran Risen Knight")
inline std::string FormatRankAndName(Game::CharacterRank rank, const std::string& name) {
    const char* rankName = Game::EnumHelpers::GetRankName(rank);
    std::string rankStr = (rankName != nullptr) ? std::string(rankName) : "";
    if (!rankStr.empty()) {
        return rankStr + " " + name;
    }
    return name; // Return just the name if the rank is Normal or unknown
}

// Helper to get full character description
inline std::string FormatCharacterSummary(Game::Profession profession, Game::Race race, uint32_t level) {
    // Validate level to prevent display of garbage values
    if (level > 100000) {
        return "Invalid Level: " + std::to_string(level);
    }
    
    const char* profName = Game::EnumHelpers::GetProfessionName(profession);
    std::string prof = (profName != nullptr) ? std::string(profName) : ("Prof ID: " + std::to_string(static_cast<uint32_t>(profession)));
    const char* raceName = Game::EnumHelpers::GetRaceName(race);
    std::string raceStr = (raceName != nullptr) ? std::string(raceName) : ("Race ID: " + std::to_string(static_cast<uint8_t>(race)));
    
    const char* armorName = Game::EnumHelpers::GetArmorWeight(profession);
    std::string armor = (armorName != nullptr) ? std::string(armorName) : "Unknown Armor";
    
    return "Lvl " + std::to_string(level) + " " + raceStr + " " + prof + " (" + armor + ")";
}

// Helper to get gadget description with context
inline std::string FormatGadgetSummary(Game::GadgetType type, bool isGatherable = true) {
    const char* gadgetName = Game::EnumHelpers::GetGadgetTypeName(type);
    std::string typeName = (gadgetName != nullptr) ? std::string(gadgetName) : ("Gadget ID: " + std::to_string(static_cast<uint32_t>(type)));
    
    if (type == Game::GadgetType::ResourceNode) {
        typeName += isGatherable ? " (Gatherable)" : " (Depleted)";
    }
    
    return typeName;
}

inline std::string ResourceNodeTypeToString(Game::ResourceNodeType type) {
    switch (type) {
	case Game::ResourceNodeType::Plant: return "Plant";
	case Game::ResourceNodeType::Tree: return "Tree";
	case Game::ResourceNodeType::Rock: return "Rock";
	case Game::ResourceNodeType::Quest: return "Quest Node";
	default:
		return "Node ID: " + std::to_string(static_cast<int>(type));
	}
}

inline const char* RarityToString(Game::ItemRarity rarity) {
    switch (rarity) {
    case Game::ItemRarity::Junk: return "Junk";
    case Game::ItemRarity::Common: return "Common";
    case Game::ItemRarity::Fine: return "Fine";
    case Game::ItemRarity::Masterwork: return "Masterwork";
    case Game::ItemRarity::Rare: return "Rare";
    case Game::ItemRarity::Exotic: return "Exotic";
    case Game::ItemRarity::Ascended: return "Ascended";
    case Game::ItemRarity::Legendary: return "Legendary";
    default: return "Unknown";
    }
}

inline const char* EquipmentSlotToString(Game::EquipmentSlot slot) {
    switch (slot) {
    case Game::EquipmentSlot::Helm: return "Helm";
    case Game::EquipmentSlot::Shoulders: return "Shoulders";
    case Game::EquipmentSlot::Chest: return "Chest";
    case Game::EquipmentSlot::Gloves: return "Gloves";
    case Game::EquipmentSlot::Pants: return "Legs";
    case Game::EquipmentSlot::Boots: return "Feet";
    case Game::EquipmentSlot::Back: return "Back";
    case Game::EquipmentSlot::Amulet: return "Amulet";
    case Game::EquipmentSlot::Accessory1: return "Accessory 1";
    case Game::EquipmentSlot::Accessory2: return "Accessory 2";
    case Game::EquipmentSlot::Ring1: return "Ring 1";
    case Game::EquipmentSlot::Ring2: return "Ring 2";
    case Game::EquipmentSlot::MainhandWeapon1: return "Weapon1 A";
    case Game::EquipmentSlot::OffhandWeapon1: return "Weapon1 B";
    case Game::EquipmentSlot::MainhandWeapon2: return "Weapon2 A";
    case Game::EquipmentSlot::OffhandWeapon2: return "Weapon2 B";
    default: return "Unknown Slot";
    }
}

} // namespace ESPFormatting
} // namespace kx
