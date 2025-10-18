#pragma once

#include "../../Utils/SafeForeignClass.h"
#include "../GameEnums.h"
#include "../offsets.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Wrapper for the game's Item Definition structure.
         * Contains the core properties of an item, like its ID and rarity.
         */
        class ItemDef : public SafeForeignClass {
        public:
            ItemDef(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetId() const {
                // Reads the unique item ID (e.g., 49371 for Quiver of Swift Flight).
                // Returns 0 if the read fails.
                return ReadMember<uint32_t>(Offsets::ItemDef::ID, 0);
            }

            Game::ItemRarity GetRarity() const {
                return ReadMember<Game::ItemRarity>(Offsets::ItemDef::RARITY, Game::ItemRarity::None);
            }
        };

    } // namespace ReClass
} // namespace kx