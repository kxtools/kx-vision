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
        class ItCliItem : public SafeForeignClass {
        public:
            ItCliItem(void* ptr) : SafeForeignClass(ptr) {}

            ItemDef GetItemDefinition() const {
                return ReadPointer<ItemDef>(Offsets::ItCliItem::ITEM_DEF);
            }

            Stat GetStatGear() const {
                return ReadPointer<Stat>(Offsets::ItCliItem::STAT_GEAR);
            }

            Stat GetStatWeapon() const {
                return ReadPointer<Stat>(Offsets::ItCliItem::STAT_WEAPON);
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

                // Calculate the base address of the embedded equipment array
                uintptr_t arrayBaseAddress = reinterpret_cast<uintptr_t>(data()) + Offsets::ChCliInventory::EQUIPMENT_ARRAY;

                // Now, safely read the pointer for the specific slot FROM the array
                void* slotPtr = nullptr;
                if (!kx::Debug::SafeRead<void*>(reinterpret_cast<void*>(arrayBaseAddress), slotIndex * sizeof(void*), slotPtr)) {
                    return ItCliItem(nullptr);
                }

                return ItCliItem(slotPtr);
            }
        };

    } // namespace ReClass
} // namespace kx