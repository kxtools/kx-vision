#pragma once

#include <cstdint>

namespace kx {
namespace Game {

// Agent and Entity Types
enum class AgentType : int {
    Character = 0,
    Gadget = 10,
    GadgetAttackTarget = 11,
    Item = 15,
    Error = -1
};

enum class AgentCategory : int {
    Character = 0,
    Dynamic = 1,
    Keyframed = 2
};

// Character Information
enum class Profession : uint32_t {
    None = 0,
    Guardian = 1,
    Warrior = 2,
    Engineer = 3,
    Ranger = 4,
    Thief = 5,
    Elementalist = 6,
    Mesmer = 7,
    Necromancer = 8,
    Revenant = 9,
    End = 10
};

enum class Race : uint8_t {
    Asura = 0,
    Charr = 1,
    Human = 2,
    Norn = 3,
    Sylvari = 4,
    None = 5
};

enum class CharacterRank : int {
    Normal = 0,
    Ambient = 1,
    Veteran = 2,
    Elite = 3,
    Champion = 4,
    Legendary = 5,
    End = 6
};

// Bitmask flags for character ranks, read directly from memory
enum class CharacterRankFlags : uint32_t {
    None = 0,
    Champion = 1 << 1,    // Unverified, from previous knowledge
    Elite = 1 << 5,    // Unverified, from previous knowledge
    Legendary = 1 << 11,   // Unverified, from previous knowledge
    Ambient = 1 << 23,   // VERIFIED from memory dump
    Veteran = 1 << 29,   // VERIFIED from memory dump
};

enum class Attitude : uint32_t {
    Friendly = 0,
    Hostile = 1,
    Indifferent = 2,
    Neutral = 3
};

// Gadget Types - Most important for ESP filtering
enum class GadgetType : uint32_t {
    Destructible = 1,       // Training dummy, siege practice targets
    Point = 2,              // PvP control points, event spawns
    Generic = 3,            // Generic, often invisible, trigger
    Crafting = 5,           // Crafting stations
    Door = 6,               // Interactive doors, gates
    BountyBoard = 11,       // Bounty boards
    Interact = 12,          // Chests, portals
    Rift = 13,              // Reality Rifts
    PlayerSpecific = 14,    // Player-specific objects
    AttackTarget = 16,      // World bosses, fort walls
    MapPortal = 17,         // Map border portals
    Waypoint = 18,          // Waypoints
    ResourceNode = 19,      // Gathering nodes, chests
    Prop = 20,              // Supply depots, anvils, jump pads
    PlayerCreated = 23,     // Turrets, siege, guild banners
    Vista = 24,             // Vistas
    BuildSite = 25,         // WvW siege build sites
    None = 26
};

enum class ResourceNodeType : int {
    Plant = 0,
    Tree = 1,
    Rock = 2,
    Quest = 3,
    None = 4
};

// Item Information
enum class ItemRarity : int {
    None = -1,
    Junk = 0,
    Common = 1,
    Fine = 2,
    Masterwork = 3,
    Rare = 4,
    Exotic = 5,
    Ascended = 6,
    Legendary = 7,
    End = 8
};

// Equipment Slots - Useful for equipment tracking
enum class EquipmentSlot : int {
    None = -1,
    AquaticHelm = 0,
    Back = 1,
    Chest = 2,
    Boots = 3,
    Gloves = 4,
    Helm = 5,
    Pants = 6,
    Shoulders = 7,

    TownChest = 14,
    TownBoots = 15,
    TownGloves = 16,
    TownHelm = 17,
    TownPants = 18,

    Accessory1 = 19,
    Accessory2 = 20,
    Ring1 = 21,
    Ring2 = 22,
    Amulet = 23,

    AquaticWeapon1 = 24,
    AquaticWeapon2 = 25,
    Novelty = 26,
    TransformWeapon = 27,

    MainhandWeapon1 = 29,
    OffhandWeapon1 = 30,
    MainhandWeapon2 = 31,
    OffhandWeapon2 = 32,
    Toy = 33,
    ForagingTool = 34,
    LoggingTool = 35,
    MiningTool = 36,

    PvpAquaticHelm = 40,
    PvpBack = 41,
    PvpChest = 42,
    PvpBoots = 43,
    PvpGloves = 44,
    PvpHelm = 45,
    PvpPants = 46,
    PvpShoulders = 47,
    PvpAquaticWeapon1 = 48,
    PvpAquaticWeapon2 = 49,
    PvpMainhandWeapon1 = 50,
    PvpOffhandWeapon1 = 51,
    PvpMainhandWeapon2 = 52,
    PvpOffhandWeapon2 = 53,

    PvpAmulet = 58,
    FishingRod = 60,
    Relic = 67,
    Backpack1 = 68,
    End = 69
};

// Weapon Types
enum class WeaponType : int {
    None = -1,
    Sword = 0,
    Hammer = 1,
    Longbow = 2,
    Shortbow = 3,
    Axe = 4,
    Dagger = 5,
    Greatsword = 6,
    Mace = 7,
    Pistol = 8,
    Rifle = 10,
    Scepter = 11,
    Staff = 12,
    Focus = 13,
    Torch = 14,
    Warhorn = 15,
    Shield = 16,
    End = 23
};

// Breakbar States - Useful for NPC status
enum class BreakbarState : int {
    Ready = 0,
    Recover = 1,
    Immune = 2,
    None = 3
};

// Combat Effects - Useful for buff/debuff tracking
enum class EffectType : uint32_t {
    None = 0,
    Protection = 717,
    Regeneration = 718,
    Swiftness = 719,
    Blind = 720,
    Crippled = 721,
    Chilled = 722,
    Poison = 723,
    Fury = 725,
    Vigor = 726,
    Immobilized = 727,
    Bleeding = 736,
    Burning = 737,
    Vulnerability = 738,
    Might = 740,
    Weakness = 742,
    Aegis = 743,
    Downed = 770,
    Fear = 791,
    Invulnerability = 848,
    Confusion = 861,
    Stun = 872,
    Retaliation = 873,
    Revealed = 890,
    Stability = 1122,
    Quickness = 1187,
    Stealth = 13017,
    Superspeed = 5974,
    Torment = 19426,
    Slow = 26766,
    Resistance = 26980,
    Alacrity = 30328
};

// Helper functions for enum conversion and validation
namespace EnumHelpers {
    
    inline const char* GetProfessionName(Profession prof) {
        switch (prof) {
            case Profession::Guardian: return "Guardian";
            case Profession::Warrior: return "Warrior";
            case Profession::Engineer: return "Engineer";
            case Profession::Ranger: return "Ranger";
            case Profession::Thief: return "Thief";
            case Profession::Elementalist: return "Elementalist";
            case Profession::Mesmer: return "Mesmer";
            case Profession::Necromancer: return "Necromancer";
            case Profession::Revenant: return "Revenant";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetRaceName(Race race) {
        switch (race) {
            case Race::Asura: return "Asura";
            case Race::Charr: return "Charr";
            case Race::Human: return "Human";
            case Race::Norn: return "Norn";
            case Race::Sylvari: return "Sylvari";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetGadgetTypeName(GadgetType type) {
        switch (type) {
            case GadgetType::ResourceNode: return "Resource Node";
            case GadgetType::Waypoint: return "Waypoint";
            case GadgetType::Vista: return "Vista";
            case GadgetType::Crafting: return "Crafting Station";
            case GadgetType::AttackTarget: return "Attack Target";
            case GadgetType::PlayerCreated: return "Player Created";
            case GadgetType::Interact: return "Interactive";
            case GadgetType::Door: return "Door";
            case GadgetType::MapPortal: return "Portal";
            case GadgetType::Destructible: return "Destructible";
            case GadgetType::Point: return "Control Point";
            case GadgetType::BountyBoard: return "Bounty Board";
            case GadgetType::Rift: return "Rift";
            case GadgetType::PlayerSpecific: return "Player Specific";
            case GadgetType::Prop: return "Prop";
            case GadgetType::BuildSite: return "Build Site";
            case GadgetType::Generic: return "Generic Trigger";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetRankName(CharacterRank rank) {
        switch (rank) {
            case CharacterRank::Normal: return "";
            case CharacterRank::Ambient: return "Ambient";
            case CharacterRank::Veteran: return "Veteran";
            case CharacterRank::Elite: return "Elite";
            case CharacterRank::Champion: return "Champion";
            case CharacterRank::Legendary: return "Legendary";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline const char* GetAttitudeName(Attitude attitude) {
        switch (attitude) {
            case Attitude::Friendly: return "Friendly";
            case Attitude::Hostile: return "Hostile";
            case Attitude::Indifferent: return "Indifferent";
            case Attitude::Neutral: return "Neutral";
            default: return nullptr; // Return nullptr for unknown, caller should handle ID display
        }
    }

    inline bool IsGatheringTool(EquipmentSlot slot) {
        return slot == EquipmentSlot::ForagingTool ||
               slot == EquipmentSlot::LoggingTool ||
               slot == EquipmentSlot::MiningTool;
    }

    inline bool IsWeaponSlot(EquipmentSlot slot) {
        return slot == EquipmentSlot::MainhandWeapon1 ||
               slot == EquipmentSlot::OffhandWeapon1 ||
               slot == EquipmentSlot::MainhandWeapon2 ||
               slot == EquipmentSlot::OffhandWeapon2;
    }

    // Check if a profession is a heavy armor class
    inline bool IsHeavyArmorProfession(Profession profession) {
        return profession == Profession::Guardian ||
               profession == Profession::Warrior;
    }

    // Check if a profession is a medium armor class
    inline bool IsMediumArmorProfession(Profession profession) {
        return profession == Profession::Engineer ||
               profession == Profession::Ranger ||
               profession == Profession::Thief;
    }

    // Check if a profession is a light armor class
    inline bool IsLightArmorProfession(Profession profession) {
        return profession == Profession::Elementalist ||
               profession == Profession::Mesmer ||
               profession == Profession::Necromancer ||
               profession == Profession::Revenant;
    }

    // Get armor weight description
    inline const char* GetArmorWeight(Profession profession) {
        if (IsHeavyArmorProfession(profession)) return "Heavy";
        if (IsMediumArmorProfession(profession)) return "Medium";
        if (IsLightArmorProfession(profession)) return "Light";
        return nullptr; // Return nullptr for unknown, caller should handle ID display
    }

    // Check if a gadget type should be considered important
    inline bool IsImportantGadgetType(GadgetType type) {
        switch (type) {
            case GadgetType::ResourceNode:
            case GadgetType::Waypoint:
            case GadgetType::Vista:
            case GadgetType::AttackTarget:
            case GadgetType::Interact:
                return true;
            default:
                return false;
        }
    }

    inline bool IsDebuff(EffectType effect) {
        switch (effect) {
            case EffectType::Blind:
            case EffectType::Crippled:
            case EffectType::Chilled:
            case EffectType::Poison:
            case EffectType::Immobilized:
            case EffectType::Bleeding:
            case EffectType::Burning:
            case EffectType::Vulnerability:
            case EffectType::Weakness:
            case EffectType::Fear:
            case EffectType::Confusion:
            case EffectType::Stun:
            case EffectType::Torment:
            case EffectType::Slow:
                return true;
            default:
                return false;
        }
    }

    inline bool IsBuff(EffectType effect) {
        switch (effect) {
            case EffectType::Protection:
            case EffectType::Regeneration:
            case EffectType::Swiftness:
            case EffectType::Fury:
            case EffectType::Vigor:
            case EffectType::Might:
            case EffectType::Aegis:
            case EffectType::Stability:
            case EffectType::Quickness:
            case EffectType::Superspeed:
            case EffectType::Resistance:
            case EffectType::Alacrity:
                return true;
            default:
                return false;
        }
    }

} // namespace EnumHelpers
} // namespace Game
} // namespace kx
