#pragma once

#include "../../Utils/SafeForeignClass.h"
#include "../offsets.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Wrapper for the game's Stat structure.
         * Contains the ID for an item's attribute combination (e.g., Berserker's).
         */
        class Stat : public SafeForeignClass {
        public:
            Stat(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetId() const {
                // Reads the stat combination ID (e.g., 599 for Berserker's).
                // Returns 0 if the read fails.
                return ReadMemberFast<uint32_t>(Offsets::Stat::ID, 0);
            }
        };

    } // namespace ReClass
} // namespace kx