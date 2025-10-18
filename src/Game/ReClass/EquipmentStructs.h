#pragma once

#include "../../Utils/SafeForeignClass.h"
#include "../offsets.h"
#include "ItemStructs.h"
#include "StatStructs.h"

namespace kx {
    namespace ReClass {

        // The total number of equipment slots in the game's data structure.
        constexpr int NUM_EQUIPMENT_SLOTS = 69;

        /**
         * @brief Wrapper for a single equipment slot.
         * Contains pointers to the item definition, stats, upgrades, etc.
         */
        class EquipSlot : public SafeForeignClass {
        public:
            EquipSlot(void* ptr) : SafeForeignClass(ptr) {}

            ItemDef GetItemDefinition() const {
                return ReadPointer<ItemDef>(Offsets::EquipSlot::ITEM_DEF);
            }

            Stat GetStatGear() const {
                return ReadPointer<Stat>(Offsets::EquipSlot::STAT_GEAR);
            }

            Stat GetStatWeapon() const {
                return ReadPointer<Stat>(Offsets::EquipSlot::STAT_WEAPON);
            }
        };

        /**
         * @brief Wrapper for the character's inventory.
         * Contains the array of equipped items.
         */
        class Inventory : public SafeForeignClass {
        public:
            Inventory(void* ptr) : SafeForeignClass(ptr) {}

            EquipSlot GetEquipSlot(int slotIndex) const {
                if (!data() || slotIndex < 0 || slotIndex >= NUM_EQUIPMENT_SLOTS) {
                    return EquipSlot(nullptr);
                }

                // Calculate the base address of the embedded equipment array
                uintptr_t arrayBaseAddress = reinterpret_cast<uintptr_t>(data()) + Offsets::Inventory::EQUIPMENT_ARRAY;

                // Now, safely read the pointer for the specific slot FROM the array
                void* slotPtr = nullptr;
                if (!kx::Debug::SafeRead<void*>(reinterpret_cast<void*>(arrayBaseAddress), slotIndex * sizeof(void*), slotPtr)) {
                    return EquipSlot(nullptr);
                }

                return EquipSlot(slotPtr);
            }
        };

    } // namespace ReClass
} // namespace kx