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
    Generic2 = 4,
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

// Attack Target Combat States
enum class AttackTargetCombatState : int32_t {
    Idle = 2,      // Idle/Inactive state
    InCombat = 3   // Active combat state
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

enum class ItemLocation : int32_t {
    None = 0,
    Agent = 1,
    Equipment = 2,
    Inventory = 3,
    InventoryAccount = 4,
    InventoryBagSlot = 5,
    InventoryOverflow = 6,
    Lootable = 7,
    Vendor = 8,
    InventoryShared = 10,
    InventoryArmory = 11,
    InventoryLegendaryArmory = 12,
    Count = 13
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

constexpr bool IsWeaponSlot(EquipmentSlot slot) {
    return slot == EquipmentSlot::MainhandWeapon1 ||
           slot == EquipmentSlot::OffhandWeapon1 ||
           slot == EquipmentSlot::MainhandWeapon2 ||
           slot == EquipmentSlot::OffhandWeapon2;
}

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

// Elite Specializations (using ArenaNet API IDs)
enum class EliteSpec : uint8_t {
    None = 0,
    Druid = 5,
    Daredevil = 7,
    Berserker = 18,
    Dragonhunter = 27,
    Reaper = 34,
    Chronomancer = 40,
    Scrapper = 43,
    Tempest = 48,
    Herald = 52,
    Soulbeast = 55,
    Weaver = 56,
    Holosmith = 57,
    Deadeye = 58,
    Mirage = 59,
    Scourge = 60,
    Spellbreaker = 61,
    Firebrand = 62,
    Renegade = 63,
    Harbinger = 64,
    Willbender = 65,
    Virtuoso = 66,
    Catalyst = 67,
    Bladesworn = 68,
    Vindicator = 69,
    Mechanist = 70,
    Specter = 71,
    Untamed = 72
};

// Mount Types
enum class MountType : uint8_t {
    None = 0,
    Jackal = 1,
    Griffon = 2,
    Springer = 3,
    Skimmer = 4,
    Raptor = 5,
    RollerBeetle = 6,
    Warclaw = 7,
    Skyscale = 8,
    Skiff = 9,
    SiegeTurtle = 10
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

} // namespace Game
} // namespace kx
