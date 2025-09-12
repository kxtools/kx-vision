#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Core/AppState.h"
#include "../../Utils/SafeForeignClass.h"
#include "../offsets.h"
#include "CharacterStructs.h"
#include "GadgetStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Character context manager - handles character and player lists
         */
        class ChCliContext : public kx::SafeForeignClass {
        public:
            ChCliContext(void* ptr) : kx::SafeForeignClass(ptr) {}

            ChCliCharacter** GetCharacterList() {
                LOG_MEMORY("ChCliContext", "GetCharacterList", data(), Offsets::CH_CLI_CONTEXT_CHARACTER_LIST);
                
                ChCliCharacter** characterList = ReadArrayPointer<ChCliCharacter*>(Offsets::CH_CLI_CONTEXT_CHARACTER_LIST);
                
                LOG_PTR("CharacterList", characterList);
                return characterList;
            }

            uint32_t GetCharacterListCapacity() {
                LOG_MEMORY("ChCliContext", "GetCharacterListCapacity", data(), Offsets::CH_CLI_CONTEXT_CHARACTER_LIST_CAPACITY);
                
                uint32_t capacity = ReadMember<uint32_t>(Offsets::CH_CLI_CONTEXT_CHARACTER_LIST_CAPACITY, 0);
                
                LOG_DEBUG("ChCliContext::GetCharacterListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            ChCliPlayer** GetPlayerList() {
                LOG_MEMORY("ChCliContext", "GetPlayerList", data(), Offsets::CH_CLI_CONTEXT_PLAYER_LIST);
                
                ChCliPlayer** playerList = ReadArrayPointer<ChCliPlayer*>(Offsets::CH_CLI_CONTEXT_PLAYER_LIST);
                
                LOG_PTR("PlayerList", playerList);
                return playerList;
            }

            uint32_t GetPlayerListSize() {
                LOG_MEMORY("ChCliContext", "GetPlayerListSize", data(), Offsets::CH_CLI_CONTEXT_PLAYER_LIST_SIZE);
                
                uint32_t size = ReadMember<uint32_t>(Offsets::CH_CLI_CONTEXT_PLAYER_LIST_SIZE, 0);
                
                LOG_DEBUG("ChCliContext::GetPlayerListSize - Size: %u", size);
                return size;
            }

            ChCliCharacter* GetLocalPlayer() {
                LOG_MEMORY("ChCliContext", "GetLocalPlayer", data(), Offsets::CH_CLI_CONTEXT_LOCAL_PLAYER);
                
                ChCliCharacter* result = ReadMember<ChCliCharacter*>(Offsets::CH_CLI_CONTEXT_LOCAL_PLAYER, nullptr);
                
                LOG_PTR("LocalPlayer", result);
                return result;
            }
        };

        /**
         * @brief Gadget context manager - handles gadget lists
         */
        class GdCliContext : public kx::SafeForeignClass {
        public:
            GdCliContext(void* ptr) : kx::SafeForeignClass(ptr) {}

            GdCliGadget** GetGadgetList() {
                LOG_MEMORY("GdCliContext", "GetGadgetList", data(), Offsets::GD_CLI_CONTEXT_GADGET_LIST);
                
                GdCliGadget** gadgetList = ReadArrayPointer<GdCliGadget*>(Offsets::GD_CLI_CONTEXT_GADGET_LIST);
                
                LOG_PTR("GadgetList", gadgetList);
                return gadgetList;
            }

            uint32_t GetGadgetListCapacity() {
                LOG_MEMORY("GdCliContext", "GetGadgetListCapacity", data(), Offsets::GD_CLI_CONTEXT_GADGET_LIST_CAPACITY);
                
                uint32_t capacity = ReadMember<uint32_t>(Offsets::GD_CLI_CONTEXT_GADGET_LIST_CAPACITY, 0);
                
                LOG_DEBUG("GdCliContext::GetGadgetListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetGadgetListCount() {
                LOG_MEMORY("GdCliContext", "GetGadgetListCount", data(), Offsets::GD_CLI_CONTEXT_GADGET_LIST_COUNT);
                
                uint32_t count = ReadMember<uint32_t>(Offsets::GD_CLI_CONTEXT_GADGET_LIST_COUNT, 0);
                
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

            ChCliContext GetChCliContext() {
                LOG_MEMORY("ContextCollection", "GetChCliContext", data(), Offsets::CONTEXT_COLLECTION_CH_CLI_CONTEXT);
                
                ChCliContext result = ReadPointer<ChCliContext>(Offsets::CONTEXT_COLLECTION_CH_CLI_CONTEXT);
                
                LOG_PTR("ChCliContext", result.data());
                return result;
            }

            GdCliContext GetGdCliContext() {
                LOG_MEMORY("ContextCollection", "GetGdCliContext", data(), Offsets::CONTEXT_COLLECTION_GD_CLI_CONTEXT);
                
                GdCliContext result = ReadPointer<GdCliContext>(Offsets::CONTEXT_COLLECTION_GD_CLI_CONTEXT);
                
                LOG_PTR("GdCliContext", result.data());
                return result;
            }
        };

    } // namespace ReClass
} // namespace kx
