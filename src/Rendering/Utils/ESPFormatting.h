#pragma once

#include <string>
#include "../../Game/GameEnums.h"

namespace kx {
namespace ESPFormatting {

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
