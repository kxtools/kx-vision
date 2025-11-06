#pragma once

#include <string>
#include "../../Game/GameEnums.h"

namespace kx {
namespace ESPFormatting {

    // --- From former EnumHelpers ---
    inline const char* GetProfessionName(Game::Profession prof) {
        switch (prof) {
            case Game::Profession::Guardian: return "Guardian";
            case Game::Profession::Warrior: return "Warrior";
            case Game::Profession::Engineer: return "Engineer";
            case Game::Profession::Ranger: return "Ranger";
            case Game::Profession::Thief: return "Thief";
            case Game::Profession::Elementalist: return "Elementalist";
            case Game::Profession::Mesmer: return "Mesmer";
            case Game::Profession::Necromancer: return "Necromancer";
            case Game::Profession::Revenant: return "Revenant";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetRaceName(Game::Race race) {
        switch (race) {
            case Game::Race::Asura: return "Asura";
            case Game::Race::Charr: return "Charr";
            case Game::Race::Human: return "Human";
            case Game::Race::Norn: return "Norn";
            case Game::Race::Sylvari: return "Sylvari";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetGadgetTypeName(Game::GadgetType type) {
        switch (type) {
            case Game::GadgetType::ResourceNode: return "Resource Node";
            case Game::GadgetType::Waypoint: return "Waypoint";
            case Game::GadgetType::Vista: return "Vista";
            case Game::GadgetType::Crafting: return "Crafting Station";
            case Game::GadgetType::AttackTarget: return "Attack Target";
            case Game::GadgetType::PlayerCreated: return "Player Created";
            case Game::GadgetType::Interact: return "Interactive";
            case Game::GadgetType::Door: return "Door";
            case Game::GadgetType::MapPortal: return "Portal";
            case Game::GadgetType::Destructible: return "Destructible";
            case Game::GadgetType::Point: return "Control Point";
            case Game::GadgetType::BountyBoard: return "Bounty Board";
            case Game::GadgetType::Rift: return "Rift";
            case Game::GadgetType::PlayerSpecific: return "Player Specific";
            case Game::GadgetType::Prop: return "Prop";
            case Game::GadgetType::BuildSite: return "Build Site";
            case Game::GadgetType::Generic: return "Generic Trigger";
            case Game::GadgetType::Generic2: return "Generic Trigger 2";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetRankName(Game::CharacterRank rank) {
        switch (rank) {
            case Game::CharacterRank::Normal: return "Normal";
            case Game::CharacterRank::Ambient: return "Ambient";
            case Game::CharacterRank::Veteran: return "Veteran";
            case Game::CharacterRank::Elite: return "Elite";
            case Game::CharacterRank::Champion: return "Champion";
            case Game::CharacterRank::Legendary: return "Legendary";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetAttitudeName(Game::Attitude attitude) {
        switch (attitude) {
            case Game::Attitude::Friendly: return "Friendly";
            case Game::Attitude::Hostile: return "Hostile";
            case Game::Attitude::Indifferent: return "Indifferent";
            case Game::Attitude::Neutral: return "Neutral";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline std::string GetAgentTypeName(Game::AgentType type) {
        switch (type) {
            case Game::AgentType::Character: return "Character";
            case Game::AgentType::Gadget: return "Gadget";
            case Game::AgentType::GadgetAttackTarget: return "Gadget Attack Target";
            case Game::AgentType::Item: return "Item";
            case Game::AgentType::Error: return "Error";
            default: return std::to_string(static_cast<int>(type));
        }
    }

    inline bool IsWeaponSlot(Game::EquipmentSlot slot) {
        return slot == Game::EquipmentSlot::MainhandWeapon1 ||
               slot == Game::EquipmentSlot::OffhandWeapon1 ||
               slot == Game::EquipmentSlot::MainhandWeapon2 ||
               slot == Game::EquipmentSlot::OffhandWeapon2;
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
