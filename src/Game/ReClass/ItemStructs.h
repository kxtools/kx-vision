#pragma once

#include "../../Utils/SafeForeignClass.h"
#include "../GameEnums.h"
#include "../offsets.h"
#include "GadgetStructs.h"

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
                return ReadMemberFast<uint32_t>(Offsets::ItemDef::ID, 0);
            }

            Game::ItemRarity GetRarity() const {
                return ReadMemberFast<Game::ItemRarity>(Offsets::ItemDef::RARITY, Game::ItemRarity::None);
            }
        };

        /**
         * @brief Wrapper for item agent structure (N0000018D)
         * Used when ItemLocation == Agent (world items)
         */
        class ItemAgentWrapper : public SafeForeignClass {
        public:
            ItemAgentWrapper(void* ptr) : SafeForeignClass(ptr) {}

            AgKeyFramed GetAgKeyFramed() const {
                return ReadPointerFast<AgKeyFramed>(Offsets::ItemAgentWrapper::AG_KEYFRAMED);
            }
        };

    } // namespace ReClass
} // namespace kx