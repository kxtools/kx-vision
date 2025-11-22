#pragma once

#include "../../Memory/SafeForeignClass.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Stat - Item stat combination structure
         * Contains the ID for an item's attribute combination (e.g., Berserker's).
         */
        class Stat : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t ID = 0x28;  // uint32_t stat combination ID
            };

        public:
            Stat(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetId() const {
                // Reads the stat combination ID (e.g., 599 for Berserker's).
                // Returns 0 if the read fails.
                return ReadMemberFast<uint32_t>(Offsets::ID, 0);
            }
        };

    } // namespace ReClass
} // namespace kx