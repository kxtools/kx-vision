#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Utils/SafeForeignClass.h"
#include "../offsets.h"
#include "CharacterStructs.h"
#include "GadgetStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Character context manager - handles character and player lists
         */
        class ChCliContext : public SafeForeignClass {
        public:
            ChCliContext(void* ptr) : SafeForeignClass(ptr) {}

            ChCliCharacter** GetCharacterList() const {
                LOG_MEMORY("ChCliContext", "GetCharacterList", data(), Offsets::ChCliContext::CHARACTER_LIST);
                
                ChCliCharacter** characterList = ReadArrayPointer<ChCliCharacter*>(Offsets::ChCliContext::CHARACTER_LIST);
                
                LOG_PTR("CharacterList", characterList);
                return characterList;
            }

            uint32_t GetCharacterListCapacity() const {
                LOG_MEMORY("ChCliContext", "GetCharacterListCapacity", data(), Offsets::ChCliContext::CHARACTER_LIST_CAPACITY);
                
                uint32_t capacity = ReadMember<uint32_t>(Offsets::ChCliContext::CHARACTER_LIST_CAPACITY, 0);
                
                LOG_DEBUG("ChCliContext::GetCharacterListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            ChCliPlayer** GetPlayerList() const {
                LOG_MEMORY("ChCliContext", "GetPlayerList", data(), Offsets::ChCliContext::PLAYER_LIST);
                
                ChCliPlayer** playerList = ReadArrayPointer<ChCliPlayer*>(Offsets::ChCliContext::PLAYER_LIST);
                
                LOG_PTR("PlayerList", playerList);
                return playerList;
            }

            uint32_t GetPlayerListSize() const {
                LOG_MEMORY("ChCliContext", "GetPlayerListSize", data(), Offsets::ChCliContext::PLAYER_LIST_CAPACITY);
                
                uint32_t size = ReadMember<uint32_t>(Offsets::ChCliContext::PLAYER_LIST_CAPACITY, 0);
                
                LOG_DEBUG("ChCliContext::GetPlayerListSize - Size: %u", size);
                return size;
            }

            ChCliCharacter* GetLocalPlayer() const {
                LOG_MEMORY("ChCliContext", "GetLocalPlayer", data(), Offsets::ChCliContext::LOCAL_PLAYER);
                
                ChCliCharacter* result = ReadMember<ChCliCharacter*>(Offsets::ChCliContext::LOCAL_PLAYER, nullptr);
                
                LOG_PTR("LocalPlayer", result);
                return result;
            }
        };

        /**
         * @brief Gadget context manager - handles gadget lists
         */
        class GdCliContext : public SafeForeignClass {
        public:
            GdCliContext(void* ptr) : SafeForeignClass(ptr) {}

            GdCliGadget** GetGadgetList() const {
                LOG_MEMORY("GdCliContext", "GetGadgetList", data(), Offsets::GdCliContext::GADGET_LIST);
                
                GdCliGadget** gadgetList = ReadArrayPointer<GdCliGadget*>(Offsets::GdCliContext::GADGET_LIST);
                
                LOG_PTR("GadgetList", gadgetList);
                return gadgetList;
            }

            uint32_t GetGadgetListCapacity() const {
                LOG_MEMORY("GdCliContext", "GetGadgetListCapacity", data(), Offsets::GdCliContext::GADGET_LIST_CAPACITY);
                
                uint32_t capacity = ReadMember<uint32_t>(Offsets::GdCliContext::GADGET_LIST_CAPACITY, 0);
                
                LOG_DEBUG("GdCliContext::GetGadgetListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetGadgetListCount() const {
                LOG_MEMORY("GdCliContext", "GetGadgetListCount", data(), Offsets::GdCliContext::GADGET_LIST_COUNT);
                
                uint32_t count = ReadMember<uint32_t>(Offsets::GdCliContext::GADGET_LIST_COUNT, 0);
                
                LOG_DEBUG("GdCliContext::GetGadgetListCount - Count: %u", count);
                return count;
            }
        };

        /**
         * @brief Root context collection - entry point for all game context access
         */
        class ContextCollection : public SafeForeignClass {
        public:
            ContextCollection(void* ptr) : SafeForeignClass(ptr) {
                // Debug: Log the ContextCollection base address using proper logging system
                if (ptr) {
                    LOG_DEBUG("ContextCollection base = 0x" + std::to_string(reinterpret_cast<uintptr_t>(ptr)));
                }
            }

            ChCliContext GetChCliContext() const {
                LOG_MEMORY("ContextCollection", "GetChCliContext", data(), Offsets::ContextCollection::CH_CLI_CONTEXT);
                
                ChCliContext result = ReadPointer<ChCliContext>(Offsets::ContextCollection::CH_CLI_CONTEXT);
                
                LOG_PTR("ChCliContext", result.data());
                return result;
            }

            GdCliContext GetGdCliContext() const {
                LOG_MEMORY("ContextCollection", "GetGdCliContext", data(), Offsets::ContextCollection::GD_CLI_CONTEXT);
                
                GdCliContext result = ReadPointer<GdCliContext>(Offsets::ContextCollection::GD_CLI_CONTEXT);
                
                LOG_PTR("GdCliContext", result.data());
                return result;
            }
        };

    } // namespace ReClass
} // namespace kx
