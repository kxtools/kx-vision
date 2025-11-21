#pragma once
#include <cstdint>

/**
 * @file offsets.h
 * @brief Memory offsets for Guild Wars 2 game structures
 * 
 * Organized into nested structs that mirror the game's class hierarchy.
 */

namespace Offsets {
    
    // ============================================================================
    // COORDINATE AND TRANSFORM STRUCTURES
    // ============================================================================

    /**
     * @brief CoChar - Character coordinate system for visual positioning
     * VISUAL_POSITION is the primary position source for real-time rendering.
     */
    struct CoChar {
        static constexpr uintptr_t VISUAL_POSITION = 0x30;  // glm::vec3 position (primary)
        static constexpr uintptr_t RIGID_BODY_PLAYER = 0x60;       // hkpRigidBody* physics rigid body (PLAYER ONLY - NPCs are nullptr) - see HavokOffsets.h
        static constexpr uintptr_t SIMPLE_CLI_WRAPPER = 0x88;   // CoCharSimpleCliWrapper* - contains additional position data and physics info
        static constexpr uintptr_t PHYSICS_PHANTOM_PLAYER = 0x100; // HkpSimpleShapePhantom* direct physics phantom pointer (PLAYER ONLY - NPCs are nullptr)
        static constexpr uintptr_t CURRENT_DIRECTION_NPC = 0x110;  // glm::vec2 current direction (NPC ONLY)
        static constexpr uintptr_t VELOCITY = 0x150;               // glm::vec3 velocity (0 when stationary, increases with movement speed) - NPC alternative velocity may exist at 0x140
        static constexpr uintptr_t BOX_SHAPE_NPC = 0x170;          // hkpBoxShape* physics box shape (NPC ONLY - Players are nullptr) - see HavokOffsets.h
        static constexpr uintptr_t CURRENT_DIRECTION_PLAYER = 0x180; // glm::vec2 current direction (PLAYER ONLY) - alternative may exist at 0x190
        static constexpr uintptr_t LOOK_ANGLE_VERTICAL_PLAYER = 0x188; // float vertical look angle/pitch (PLAYER ONLY) - 1.0 when looking up, -1.0 when looking down (only updates when moving)
    };

    /**
     * @brief CoCharSimpleCliWrapper intermediate object accessed via CoChar->0x88
     * Note: PHYSICS_PHANTOM_PLAYER and BOX_SHAPE_NPC are entity-type specific.
     * Havok physics offsets are in HavokOffsets.h
     */
    struct CoCharSimpleCliWrapper {
        static constexpr uintptr_t POSITION_ALT1 = 0xB8;    // glm::vec3 alternative position 1
        static constexpr uintptr_t POSITION_ALT2 = 0x118;   // glm::vec3 alternative position 2 (may lag)
        static constexpr uintptr_t PHYSICS_PHANTOM_PLAYER = 0x78;  // hkpSimpleShapePhantom* physics object (PLAYER ONLY) - see HavokOffsets.h
        static constexpr uintptr_t BOX_SHAPE_NPC = 0xE8;        // hkpBoxShape* physics box shape (NPC ONLY - Players are nullptr) - see HavokOffsets.h
    };

    /**
     * @brief CoKeyframed - Coordinate system for keyframed objects (gadgets)
     */
    struct CoKeyframed {
        static constexpr uintptr_t POSITION = 0x0030;  // glm::vec3 position
        static constexpr uintptr_t RIGID_BODY = 0x0060; // hkpRigidBody* physics rigid body (gadgets only) - see HavokOffsets.h
        static constexpr uintptr_t ROTATION = 0x00F8; // glm::vec2 rotation (gadget rotation)
    };

    // ============================================================================
    // AGENT STRUCTURES
    // ============================================================================

    /**
     * @brief AgChar - Agent wrapper for characters
     */
    struct AgChar {
        static constexpr uintptr_t CO_CHAR = 0x50;  // CoChar* coordinate system
        static constexpr uintptr_t TYPE = 0x08;     // int32_t agent type identifier
        static constexpr uintptr_t ID = 0x0C;       // int32_t agent ID
        static constexpr uintptr_t GROUNDED_POSITION32 = 0x120;  // glm::vec3 last grounded/navmesh position (scaled by 32)
    };

    /**
     * @brief AgKeyframed - Agent wrapper for keyframed objects (gadgets)
     * 
     * TYPE values:
     * - 10: Regular gadget (AgentType::Gadget)
     * - 11: Attack target (AgentType::GadgetAttackTarget) - walls, destructible objects
     */
    struct AgKeyframed {
        static constexpr uintptr_t TYPE = 0x08;            // int32_t agent type identifier
        static constexpr uintptr_t ID = 0x0C;              // int32_t agent ID
        static constexpr uintptr_t GADGET_TYPE = 0x40;     // uint32_t gadget type
        static constexpr uintptr_t CO_KEYFRAMED = 0x0050;  // CoKeyframed* coordinate system
    };

    /**
     * @brief AgentInl - Internal agent structure for attack targets
     * 
     * Internal class: Gw2::Engine::Agent::AgentInl
     * Used in the attack target list (walls, destructible objects, etc.)
     * Entries point to AgKeyframed with TYPE=11 (GadgetAttackTarget)
     */
    struct AgentInl {
        static constexpr uintptr_t AG_KEYFRAMED = 0x18;    // AgKeyframed* agent wrapper
        static constexpr uintptr_t COMBAT_STATE = 0x0034;  // int32_t combat state flag (2=Idle, 3=In Combat) [CONFIRMED]
    };

    // ============================================================================
    // CHARACTER SUBSYSTEMS
    // ============================================================================

    /**
     * @brief ChCliHealth - Character health management
     */
    struct ChCliHealth {
        static constexpr uintptr_t CURRENT = 0x0C;  // float current health
        static constexpr uintptr_t MAX = 0x10;      // float maximum health
        static constexpr uintptr_t HEALTH_REGEN_RATE = 0x14; // float health regeneration rate (0 in combat, often 10% of max HP otherwise)
        static constexpr uintptr_t BARRIER = 0x28;  // float current barrier
    };

    /**
     * @brief ChCliEnergies - Character mount/special energy management
     */
    struct ChCliEnergies {
        static constexpr uintptr_t CURRENT = 0x0C;  // float current energy
        static constexpr uintptr_t MAX = 0x10;      // float maximum energy
    };

    /**
     * @brief ChCliEndurance - Character dodge/endurance management
     */
    struct ChCliEndurance {
        static constexpr uintptr_t CURRENT = 0x10;  // float current endurance
        static constexpr uintptr_t MAX = 0x14;      // float maximum endurance
        // Note: A second pool might exist at offsets 0x18/0x20
    };

    /**
     * @brief ChCliSkillbar - Character skillbar management
     */
    struct ChCliSkillbar {
        static constexpr uintptr_t SKILL_TRIGGER_BIT = 0xB0;  // uint32_t momentary indicator for which skill was just activated (always holds exactly one bit or zero)
        static constexpr uintptr_t SKILLS_ARRAY = 0x1D0;      // array of skills
    };

    /**
     * @brief CmbtCliBreakBar - Combat breakbar management
     */
    struct CmbtCliBreakBar {
        static constexpr uintptr_t STATE = 0x40;    // int32_t breakbar state (0=active, 1=regenerating)
        static constexpr uintptr_t CURRENT = 0x44;  // float current breakbar (range: 1.0 to 0.0)
    };

    /**
     * @brief ChCliCoreStats - Character core statistics (race, level, profession)
     */
    struct ChCliCoreStats {
        static constexpr uintptr_t RACE = 0x33;          // uint8_t race ID
        static constexpr uintptr_t LEVEL = 0xAC;         // uint32_t actual level
        static constexpr uintptr_t PROFESSION = 0x12C;   // uint32_t profession ID
        static constexpr uintptr_t SCALED_LEVEL = 0x234; // uint32_t scaled/effective level
    };

    // ============================================================================
    // EQUIPMENT AND INVENTORY
    // ============================================================================

    /**
     * @brief Stat - Item stat combination structure
     */
    struct Stat {
        static constexpr uintptr_t ID = 0x28;  // uint32_t stat combination ID
    };

    /**
     * @brief ItemDef - Item definition with ID and rarity
     */
    struct ItemDef {
        static constexpr uintptr_t ID = 0x28;      // uint32_t item ID
        static constexpr uintptr_t RARITY = 0x60;  // uint32_t rarity level
        static constexpr uintptr_t TEXT_NAME_ID = 0x80;       // uint32_t text ID for the item name
    };

    /**
     * @brief ItCliItem - Equipment slot containing item and stat data
     */
    struct ItCliItem {
        static constexpr uintptr_t ITEM_DEF = 0x40;      // ItemDef* item definition
        static constexpr uintptr_t LOCATION_TYPE = 0x48;  // uint16_t location type (mask with 0xF)
        static constexpr uintptr_t ITEM_AGENT = 0x58;     // ItemAgentWrapper* pointer to item agent (for world items)
        static constexpr uintptr_t STAT_GEAR = 0xA0;     // Stat* for armor/trinkets
        static constexpr uintptr_t STAT_WEAPON = 0xA8;   // Stat* for weapons
        static constexpr uintptr_t LOOTABLE = 0x88;      // Pointer to Lootable wrapper (future use)
        
        // Historical/Unverified offsets from old GearCheck - require verification
        // static constexpr uintptr_t RUNE = 0xC0;    // Rune* upgrade
        // static constexpr uintptr_t SIGIL1 = 0xC8;  // Sigil* first weapon sigil
        // static constexpr uintptr_t SIGIL2 = 0xD0;  // Sigil* second weapon sigil
    };

    /**
     * @brief ItemAgentWrapper - Wrapper for item agent structure (N0000018D)
     * Used when ItemLocation == Agent (world items)
     */
    struct ItemAgentWrapper {
        static constexpr uintptr_t AG_KEYFRAMED = 0x18;  // AgKeyframed* agent wrapper
    };

    /**
     * @brief ChCliInventory - Character inventory container
     */
    struct ChCliInventory {
        static constexpr uintptr_t EQUIPMENT_ARRAY = 0x160;  // ItCliItem** array of equipment slots
    };

    // ============================================================================
    // CHARACTER MAIN STRUCTURE
    // ============================================================================

    /**
     * @brief ChCliCharacter - Main character structure containing all subsystems
     */
    struct ChCliCharacter {
        static constexpr uintptr_t AGENT = 0x98;            // AgChar* character's agent
        static constexpr uintptr_t ATTITUDE = 0x00C0;       // uint32_t attitude flags
        static constexpr uintptr_t BREAKBAR = 0x00C8;       // CmbtCliBreakBar* breakbar subsystem
        static constexpr uintptr_t RANK_FLAGS = 0x0264;     // uint32_t rank flags (veteran, elite, etc.)
        static constexpr uintptr_t CORE_STATS = 0x0388;     // ChCliCoreStats* stats subsystem
        static constexpr uintptr_t ENDURANCE = 0x03D0;      // ChCliEndurance* dodge/endurance subsystem
        static constexpr uintptr_t ENERGIES = 0x03D8;       // ChCliEnergies* mount/special energy subsystem
        static constexpr uintptr_t FORCE = 0x03E0;          // ChCliForce* force subsystem
        static constexpr uintptr_t HEALTH = 0x03E8;         // ChCliHealth* health subsystem
        static constexpr uintptr_t INVENTORY = 0x3F0;       // ChCliInventory* inventory subsystem
        static constexpr uintptr_t SKILLBAR = 0x0520;       // ChCliSkillbar* skillbar subsystem
    };

    /**
     * @brief ChCliPlayer - Player wrapper containing character and name
     */
    struct ChCliPlayer {
        static constexpr uintptr_t CHARACTER_PTR = 0x18;  // ChCliCharacter* player's character
        static constexpr uintptr_t NAME_PTR = 0x68;       // wchar_t* player name string
    };

    // ============================================================================
    // GADGET STRUCTURES
    // ============================================================================

    /**
     * @brief GdCliHealth - Gadget health management
     */
    struct GdCliHealth {
        static constexpr uintptr_t CURRENT = 0x0C;  // float current health
        static constexpr uintptr_t MAX = 0x10;      // float maximum health
    };

    /**
     * @brief GdCliGadget - Game gadget/object structure
     */
    struct GdCliGadget {
        static constexpr uintptr_t AG_KEYFRAMED = 0x0038;         // AgKeyframed* agent wrapper
        static constexpr uintptr_t TYPE = 0x0208;                 // uint32_t gadget type
        static constexpr uintptr_t HEALTH = 0x0220;               // GdCliHealth* health subsystem
        static constexpr uintptr_t RESOURCE_NODE_TYPE = 0x04EC;   // uint32_t resource node type
        static constexpr uintptr_t FLAGS = 0x04F0;                // uint32_t gadget flags
        
        // Gadget flag constants
        static constexpr uint32_t FLAG_GATHERABLE = 0x2;  // Indicates gatherable resource
    };

    // ============================================================================
    // CONTEXT MANAGEMENT STRUCTURES
    // ============================================================================

    /**
     * @brief ChCliContext - Character context managing all characters and players
     * 
     * Note: CAPACITY/COUNT are element counts (not bytes), represent zone limits not visible entities.
     *       CAPACITY >= COUNT always. Arrays are sparse - use CAPACITY for iteration, validate pointers.
     */
    struct ChCliContext {
        static constexpr uintptr_t CHARACTER_LIST = 0x60;          // ChCliCharacter** array
        static constexpr uintptr_t CHARACTER_LIST_CAPACITY = 0x68; // uint32_t capacity (element count)
        static constexpr uintptr_t CHARACTER_LIST_COUNT = 0x6C;    // uint32_t count (element count)
        static constexpr uintptr_t PLAYER_LIST = 0x80;             // ChCliPlayer** array
        static constexpr uintptr_t PLAYER_LIST_CAPACITY = 0x88;    // uint32_t capacity (element count)
        static constexpr uintptr_t PLAYER_LIST_COUNT = 0x8C;       // uint32_t count (element count)
        static constexpr uintptr_t LOCAL_PLAYER = 0x98;            // ChCliCharacter* local player
    };

    /**
     * @brief GdCliContext - Gadget context managing all gadgets/objects
     * 
     * Note: CAPACITY/COUNT are element counts (not bytes), represent zone limits not visible entities.
     *       CAPACITY >= COUNT always. Arrays are sparse - use CAPACITY for iteration, validate pointers.
     */
    struct GdCliContext {
        static constexpr uintptr_t GADGET_LIST = 0x0030;          // GdCliGadget** array
        static constexpr uintptr_t GADGET_LIST_CAPACITY = 0x0038; // uint32_t capacity (element count)
        static constexpr uintptr_t GADGET_LIST_COUNT = 0x003C;    // uint32_t count (element count)
        
        // Attack target list (walls, destructible objects, etc.)
        // Internal class: Gw2::Engine::Agent::AgentInl
        // Entries are AgentInl structures pointing to AgKeyframed with TYPE=11 (GadgetAttackTarget)
        static constexpr uintptr_t ATTACK_TARGET_LIST = 0x0010;          // AgentInl** array
        static constexpr uintptr_t ATTACK_TARGET_LIST_CAPACITY = 0x0018; // uint32_t capacity (element count)
        static constexpr uintptr_t ATTACK_TARGET_LIST_COUNT = 0x001C;    // uint32_t count (element count)
    };

    /**
     * @brief ItCliContext - Item context managing all items
     * 
     * Note: CAPACITY/COUNT are element counts (not bytes), represent zone limits not visible entities.
     *       CAPACITY >= COUNT always. Arrays are sparse - use CAPACITY for iteration, validate pointers.
     */
    struct ItCliContext {
        static constexpr uintptr_t ITEM_LIST = 0x30;          // ItCliItem** array
        static constexpr uintptr_t ITEM_LIST_CAPACITY = 0x38; // uint32_t capacity (element count)
        static constexpr uintptr_t ITEM_LIST_COUNT = 0x3C;    // uint32_t count (element count)
    };

    /**
     * @brief ContextCollection - Root collection containing all context managers
     */
    struct ContextCollection {
        static constexpr uintptr_t CH_CLI_CONTEXT = 0x98;   // ChCliContext* character context
        static constexpr uintptr_t GD_CLI_CONTEXT = 0x0138; // GdCliContext* gadget context
        static constexpr uintptr_t IT_CLI_CONTEXT = 0x0178; // ItCliContext* item context
    };

} // namespace Offsets
