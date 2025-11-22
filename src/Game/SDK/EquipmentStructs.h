#pragma once

#include "../../Memory/SafeForeignClass.h"
#include "../GameEnums.h"
#include "ItemStructs.h"
#include "StatStructs.h"
#include "GadgetStructs.h" 

namespace kx {
    namespace ReClass {

        // The total number of equipment slots in the game's data structure.
        constexpr int NUM_EQUIPMENT_SLOTS = 69;

        /**
         * @brief ItCliItem - Equipment slot containing item and stat data
         * Contains pointers to the item definition, stats, upgrades, etc.
         */
        class ItCliItem : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t ITEM_DEF = 0x40;      // ItemDef* item definition
                static constexpr uintptr_t LOCATION_TYPE = 0x48;  // uint16_t location type (mask with 0xF)

                /**
                 * @brief DATA_PTR - Polymorphic pointer based on LOCATION_TYPE
                 * - Location::Agent (1)        -> AgentInl* (Item on ground)
                 * - Location::Inventory (3)    -> ChCliInventory* (Item in bag)
                 * - Location::Equipment (2)    -> ChCliInventory* (Item equipped)
                 * - Location::Lootable (7)     -> LootCliLootable*
                 * - Location::Vendor (8)       -> VendCliVendor*
                 */
                static constexpr uintptr_t DATA_PTR = 0x58;

                static constexpr uintptr_t STAT_GEAR = 0xA0;     // Stat* for armor/trinkets
                static constexpr uintptr_t STAT_WEAPON = 0xA8;   // Stat* for weapons
            };

        public:
            ItCliItem(void* ptr) : SafeForeignClass(ptr) {}

            ItemDef GetItemDefinition() const {
                return ReadPointerFast<ItemDef>(Offsets::ITEM_DEF);
            }

            Game::ItemLocation GetLocationType() const {
                uint16_t raw = ReadMemberFast<uint16_t>(Offsets::LOCATION_TYPE, 0);
                return static_cast<Game::ItemLocation>(raw & 0xF);
            }

            /**
             * @brief Gets the raw data pointer at 0x58.
             * The type of this data depends on GetLocationType().
             */
            void* GetDataPtr() const {
                return ReadMemberFast<void*>(Offsets::DATA_PTR, nullptr);
            }

            /**
             * @brief Safe accessor for items on the ground (Location == Agent).
             * @return AgentInl wrapper if location is Agent, otherwise nullptr wrapper.
             */
            AgentInl GetAsAgent() const {
                if (GetLocationType() == Game::ItemLocation::Agent) {
                    return ReadPointerFast<AgentInl>(Offsets::DATA_PTR);
                }
                return AgentInl(nullptr);
            }

            // Note: We could add GetAsInventory() here for cases where Location == Inventory,
            // but that would require including CharacterStructs.h which creates a circular
            // dependency (since ChCliCharacter contains ChCliInventory which contains ItCliItem).
            // For now, use GetDataPtr() and cast externally if needed.

            Stat GetStatGear() const {
                return ReadPointerFast<Stat>(Offsets::STAT_GEAR);
            }

            Stat GetStatWeapon() const {
                return ReadPointerFast<Stat>(Offsets::STAT_WEAPON);
            }
        };

        /**
         * @brief ChCliInventory - Character inventory container
         * Contains the array of equipped items.
         */
        class ChCliInventory : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t EQUIPMENT_ARRAY = 0x160;  // ItCliItem** array of equipment slots
            };

        public:
            ChCliInventory(void* ptr) : SafeForeignClass(ptr) {}

            ItCliItem GetEquipSlot(int slotIndex) const {
                if (!data() || slotIndex < 0 || slotIndex >= NUM_EQUIPMENT_SLOTS) {
                    return ItCliItem(nullptr);
                }

                uintptr_t arrayBaseAddress = reinterpret_cast<uintptr_t>(data()) + Offsets::EQUIPMENT_ARRAY;
                auto slotArray = reinterpret_cast<void**>(arrayBaseAddress);
                return ItCliItem(slotArray[slotIndex]);
            }
        };

    } // namespace ReClass
} // namespace kx