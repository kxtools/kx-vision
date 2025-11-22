#pragma once

#include "../../Utils/SafeForeignClass.h"
#include "../GameEnums.h"
#include "../offsets.h"
#include "ItemStructs.h"
#include "StatStructs.h"
#include "GadgetStructs.h" 

namespace kx {
    namespace ReClass {

        // The total number of equipment slots in the game's data structure.
        constexpr int NUM_EQUIPMENT_SLOTS = 69;

        /**
         * @brief Wrapper for a single equipment slot.
         * Contains pointers to the item definition, stats, upgrades, etc.
         */
        class ItCliItem : public SafeForeignClass {
        public:
            ItCliItem(void* ptr) : SafeForeignClass(ptr) {}

            ItemDef GetItemDefinition() const {
                return ReadPointerFast<ItemDef>(Offsets::ItCliItem::ITEM_DEF);
            }

            Game::ItemLocation GetLocationType() const {
                uint16_t raw = ReadMemberFast<uint16_t>(Offsets::ItCliItem::LOCATION_TYPE, 0);
                return static_cast<Game::ItemLocation>(raw & 0xF);
            }

            /**
             * @brief Gets the raw data pointer at 0x58.
             * The type of this data depends on GetLocationType().
             */
            void* GetDataPtr() const {
                return ReadMemberFast<void*>(Offsets::ItCliItem::DATA_PTR, nullptr);
            }

            /**
             * @brief Safe accessor for items on the ground (Location == Agent).
             * @return AgentInl wrapper if location is Agent, otherwise nullptr wrapper.
             */
            AgentInl GetAsAgent() const {
                if (GetLocationType() == Game::ItemLocation::Agent) {
                    return ReadPointerFast<AgentInl>(Offsets::ItCliItem::DATA_PTR);
                }
                return AgentInl(nullptr);
            }

            // Note: We could add GetAsInventory() here for cases where Location == Inventory,
            // but that would require including CharacterStructs.h which creates a circular
            // dependency (since ChCliCharacter contains ChCliInventory which contains ItCliItem).
            // For now, use GetDataPtr() and cast externally if needed.

            Stat GetStatGear() const {
                return ReadPointerFast<Stat>(Offsets::ItCliItem::STAT_GEAR);
            }

            Stat GetStatWeapon() const {
                return ReadPointerFast<Stat>(Offsets::ItCliItem::STAT_WEAPON);
            }
        };

        /**
         * @brief Wrapper for the character's inventory.
         * Contains the array of equipped items.
         */
        class ChCliInventory : public SafeForeignClass {
        public:
            ChCliInventory(void* ptr) : SafeForeignClass(ptr) {}

            ItCliItem GetEquipSlot(int slotIndex) const {
                if (!data() || slotIndex < 0 || slotIndex >= NUM_EQUIPMENT_SLOTS) {
                    return ItCliItem(nullptr);
                }

                uintptr_t arrayBaseAddress = reinterpret_cast<uintptr_t>(data()) + Offsets::ChCliInventory::EQUIPMENT_ARRAY;
                auto slotArray = reinterpret_cast<void**>(arrayBaseAddress);
                return ItCliItem(slotArray[slotIndex]);
            }
        };

    } // namespace ReClass
} // namespace kx