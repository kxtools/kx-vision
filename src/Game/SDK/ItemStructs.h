#pragma once

#include "../../Memory/SafeForeignClass.h"
#include "../GameEnums.h"
#include "GadgetStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief ItemDef - Item definition with ID and rarity
         * Contains the core properties of an item, like its ID and rarity.
         */
        class ItemDef : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t ID = 0x28;      // uint32_t item ID
                static constexpr uintptr_t RARITY = 0x60;  // uint32_t rarity level
                static constexpr uintptr_t TEXT_NAME_ID = 0x80;       // uint32_t text ID for the item name
            };

        public:
            ItemDef(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetId() const {
                // Reads the unique item ID (e.g., 49371 for Quiver of Swift Flight).
                // Returns 0 if the read fails.
                return ReadMemberFast<uint32_t>(Offsets::ID, 0);
            }

            Game::ItemRarity GetRarity() const {
                return ReadMemberFast<Game::ItemRarity>(Offsets::RARITY, Game::ItemRarity::None);
            }
        };

    } // namespace ReClass
} // namespace kx