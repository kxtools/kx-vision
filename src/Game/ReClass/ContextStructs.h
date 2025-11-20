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

            uint32_t GetCharacterListCount() const {
                LOG_MEMORY("ChCliContext", "GetCharacterListCount", data(), Offsets::ChCliContext::CHARACTER_LIST_COUNT);
                
                uint32_t count = ReadMember<uint32_t>(Offsets::ChCliContext::CHARACTER_LIST_COUNT, 0);
                
                LOG_DEBUG("ChCliContext::GetCharacterListCount - Count: %u", count);
                return count;
            }

            ChCliPlayer** GetPlayerList() const {
                LOG_MEMORY("ChCliContext", "GetPlayerList", data(), Offsets::ChCliContext::PLAYER_LIST);
                
                ChCliPlayer** playerList = ReadArrayPointer<ChCliPlayer*>(Offsets::ChCliContext::PLAYER_LIST);
                
                LOG_PTR("PlayerList", playerList);
                return playerList;
            }

            uint32_t GetPlayerListCapacity() const {
                LOG_MEMORY("ChCliContext", "GetPlayerListCapacity", data(), Offsets::ChCliContext::PLAYER_LIST_CAPACITY);
                
                uint32_t capacity = ReadMember<uint32_t>(Offsets::ChCliContext::PLAYER_LIST_CAPACITY, 0);
                
                LOG_DEBUG("ChCliContext::GetPlayerListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetPlayerListCount() const {
                LOG_MEMORY("ChCliContext", "GetPlayerListCount", data(), Offsets::ChCliContext::PLAYER_LIST_COUNT);
                
                uint32_t count = ReadMember<uint32_t>(Offsets::ChCliContext::PLAYER_LIST_COUNT, 0);
                
                LOG_DEBUG("ChCliContext::GetPlayerListCount - Count: %u", count);
                return count;
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

            AgentInl** GetAttackTargetList() const {
                LOG_MEMORY("GdCliContext", "GetAttackTargetList", data(), Offsets::GdCliContext::ATTACK_TARGET_LIST);
                
                AgentInl** attackTargetList = ReadArrayPointer<AgentInl*>(Offsets::GdCliContext::ATTACK_TARGET_LIST);
                
                LOG_PTR("AttackTargetList", attackTargetList);
                return attackTargetList;
            }

            uint32_t GetAttackTargetListCapacity() const {
                LOG_MEMORY("GdCliContext", "GetAttackTargetListCapacity", data(), Offsets::GdCliContext::ATTACK_TARGET_LIST_CAPACITY);
                
                uint32_t capacity = ReadMember<uint32_t>(Offsets::GdCliContext::ATTACK_TARGET_LIST_CAPACITY, 0);
                
                LOG_DEBUG("GdCliContext::GetAttackTargetListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetAttackTargetListCount() const {
                LOG_MEMORY("GdCliContext", "GetAttackTargetListCount", data(), Offsets::GdCliContext::ATTACK_TARGET_LIST_COUNT);
                
                uint32_t count = ReadMember<uint32_t>(Offsets::GdCliContext::ATTACK_TARGET_LIST_COUNT, 0);
                
                LOG_DEBUG("GdCliContext::GetAttackTargetListCount - Count: %u", count);
                return count;
            }
        };

        /**
         * @brief World context wrapper - contains global world time
         */
        class AgWorld : public SafeForeignClass {
        public:
            AgWorld(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetWorldTime() const {
                LOG_MEMORY("AgWorld", "GetWorldTime", data(), Offsets::AgWorld::WORLD_TIME);
                return ReadMember<uint32_t>(Offsets::AgWorld::WORLD_TIME, 0);
            }
        };

        /**
         * @brief Global API context wrapper - provides access to world context
         */
        class AgApi : public SafeForeignClass {
        public:
            AgApi(void* ptr) : SafeForeignClass(ptr) {}

            AgWorld GetAgWorld() const {
                LOG_MEMORY("AgApi", "GetAgWorld", data(), Offsets::AgApi::AG_WORLD);
                AgWorld result = ReadPointer<AgWorld>(Offsets::AgApi::AG_WORLD);
                LOG_PTR("AgWorld", result.data());
                return result;
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

            AgApi GetAgApi() const {
                LOG_MEMORY("ContextCollection", "GetAgApi", data(), Offsets::ContextCollection::AG_API);
                AgApi result = ReadPointer<AgApi>(Offsets::ContextCollection::AG_API);
                LOG_PTR("AgApi", result.data());
                return result;
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
