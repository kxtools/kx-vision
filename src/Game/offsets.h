#pragma once
#include <cstdint>

/**
 * @file offsets.h
 * @brief Memory offsets for Guild Wars 2 game structures
 * 
 * This file organizes memory offsets into nested structs that mirror the game's
 * class hierarchy. This structure provides better maintainability and makes the
 * relationship between game classes and their members more explicit.
 * 
 * Usage Examples:
 *   Offsets::ChCliCharacter::AGENT        // Character's agent pointer
 *   Offsets::ChCliHealth::CURRENT         // Health current value
 *   Offsets::ContextCollection::CH_CLI_CONTEXT  // Character context in collection
 * 
 * @note All offsets are organized by game class for clarity and maintainability.
 *       The structured approach makes relationships between classes explicit.
 */

namespace Offsets {
    
    // ============================================================================
    // COORDINATE AND TRANSFORM STRUCTURES
    // ============================================================================

    /**
     * @brief CoChar - Character coordinate system for visual positioning
     * 
     * VISUAL_POSITION (0x30) is the primary position source and works well.
     * Testing shows it updates smoothly and accurately for real-time rendering.
     */
    struct CoChar {
        static constexpr uintptr_t VISUAL_POSITION = 0x30;  // glm::vec3 position (primary - TESTED: Good, smooth updates)
        static constexpr uintptr_t UNKNOWN_OBJECT = 0x88;   // Unknown* - contains additional position data
        static constexpr uintptr_t PHYSICS_PHANTOM = 0x100; // HkpSimpleShapePhantom* direct physics phantom pointer
    };

    /**
     * @brief Unknown intermediate object accessed via CoChar->0x88
     * Contains alternative position sources that may update at different rates
     * 
     * TEST RESULTS:
     * - POSITION_ALT1 (0xB8): Updates similarly to Primary - smooth and accurate
     * - POSITION_ALT2 (0x118): LAGS BEHIND - visual delay, not recommended
     * - PHYSICS_PHANTOM->POSITION (0x78->0x120): Updates similarly to Primary - smooth and accurate
     * 
     * RECOMMENDATION: Use Primary (CoChar::VISUAL_POSITION) or Alt1 for best results
     * Primary, Alt1, and Physics all update at similar rates with good smoothness
     */
    struct CoCharUnknown {
        static constexpr uintptr_t POSITION_ALT1 = 0xB8;    // glm::vec3 alternative position 1 (TESTED: Good, similar to Primary)
        static constexpr uintptr_t POSITION_ALT2 = 0x118;   // glm::vec3 alternative position 2 (TESTED: Lags behind, not recommended)
        static constexpr uintptr_t PHYSICS_PHANTOM = 0x78;  // hkpSimpleShapePhantom* physics object (TESTED: Good, similar to Primary)
    };

    /**
     * @brief hkpSimpleShapePhantom - Havok physics phantom object
     * Contains physics-driven position that updates similarly to Primary position.
     * 
     * TESTED: Physics position (0x120) updates smoothly and accurately, similar to Primary.
     * Can be used as alternative to Primary position if needed.
     */
    struct HkpSimpleShapePhantom {
        static constexpr uintptr_t PHYSICS_POSITION = 0x120;  // glm::vec3 physics position (TESTED: Good, similar to Primary)
    };

    /**
     * @brief CoKeyframed - Coordinate system for keyframed objects (gadgets)
     */
    struct CoKeyframed {
        static constexpr uintptr_t POSITION = 0x0030;  // glm::vec3 position
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
     */
    struct AgKeyframed {
        static constexpr uintptr_t TYPE = 0x08;            // int32_t agent type identifier
        static constexpr uintptr_t ID = 0x0C;              // int32_t agent ID
        static constexpr uintptr_t GADGET_TYPE = 0x40;     // uint32_t gadget type
        static constexpr uintptr_t CO_KEYFRAMED = 0x0050;  // CoKeyframed* coordinate system
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
     * @brief ChCliSpecialEnergies - Character mount/special energy management
     */
    struct ChCliSpecialEnergies {
        static constexpr uintptr_t CURRENT = 0x0C;  // float current energy
        static constexpr uintptr_t MAX = 0x10;      // float maximum energy
    };

    /**
     * @brief ChCliEnergies - Character dodge/endurance management
     */
    struct ChCliEnergies {
        static constexpr uintptr_t CURRENT = 0x10;  // float current endurance
        static constexpr uintptr_t MAX = 0x14;      // float maximum endurance
        // Note: A second pool might exist at offsets 0x18/0x20
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
     * @brief EquipSlot - Equipment slot containing item and stat data
     */
    struct EquipSlot {
        static constexpr uintptr_t ITEM_DEF = 0x40;      // ItemDef* item definition
        static constexpr uintptr_t STAT_GEAR = 0xA0;     // Stat* for armor/trinkets
        static constexpr uintptr_t STAT_WEAPON = 0xA8;   // Stat* for weapons
        
        // Historical/Unverified offsets from old GearCheck - require verification
        // static constexpr uintptr_t RUNE = 0xC0;    // Rune* upgrade
        // static constexpr uintptr_t SIGIL1 = 0xC8;  // Sigil* first weapon sigil
        // static constexpr uintptr_t SIGIL2 = 0xD0;  // Sigil* second weapon sigil
    };

    /**
     * @brief Inventory - Character inventory container
     */
    struct Inventory {
        static constexpr uintptr_t EQUIPMENT_ARRAY = 0x160;  // EquipSlot** array of equipment slots
    };

    // ============================================================================
    // CHARACTER MAIN STRUCTURE
    // ============================================================================

    /**
     * @brief ChCliCharacter - Main character structure containing all subsystems
     */
    struct ChCliCharacter {
        static constexpr uintptr_t AGENT = 0x98;          // AgChar* character's agent
        static constexpr uintptr_t ATTITUDE = 0x00C0;     // uint32_t attitude flags
        static constexpr uintptr_t RANK_FLAGS = 0x0264;   // uint32_t rank flags (veteran, elite, etc.)
        static constexpr uintptr_t CORE_STATS = 0x0388;   // ChCliCoreStats* stats subsystem
        static constexpr uintptr_t ENERGIES = 0x03D0;     // ChCliEnergies* dodge/endurance subsystem
        static constexpr uintptr_t SPECIAL_ENERGIES = 0x03D8; // ChCliSpecialEnergies* mount/special energy subsystem
        static constexpr uintptr_t HEALTH = 0x03E8;       // ChCliHealth* health subsystem
        static constexpr uintptr_t INVENTORY = 0x3F0;     // Inventory* inventory subsystem
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
     * @brief GdCliGadget - Game gadget/object structure
     */
    struct GdCliGadget {
        static constexpr uintptr_t AG_KEYFRAMED = 0x0038;         // AgKeyframed* agent wrapper
        static constexpr uintptr_t TYPE = 0x0208;                 // uint32_t gadget type
        static constexpr uintptr_t HEALTH = 0x0220;               // ChCliHealth* health subsystem
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
    };

    /**
     * @brief ContextCollection - Root collection containing all context managers
     */
    struct ContextCollection {
        static constexpr uintptr_t CH_CLI_CONTEXT = 0x98;   // ChCliContext* character context
        static constexpr uintptr_t GD_CLI_CONTEXT = 0x0138; // GdCliContext* gadget context
    };

} // namespace Offsets
