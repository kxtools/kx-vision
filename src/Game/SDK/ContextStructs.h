#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Memory/ForeignClass.h"
#include "../../Memory/SafeGameArray.h"
#include "CharacterStructs.h"
#include "GadgetStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief ChCliContext - Character context managing all characters and players
         *
         * Note: CAPACITY/COUNT are element counts (not bytes), represent zone limits not visible entities.
         *       CAPACITY >= COUNT always. Arrays are sparse - use CAPACITY for iteration, validate pointers.
         */
        class ChCliContext : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CHARACTER_LIST = 0x60;          // ChCliCharacter** array
                static constexpr uintptr_t CHARACTER_LIST_CAPACITY = 0x68; // uint32_t capacity (element count)
                static constexpr uintptr_t CHARACTER_LIST_COUNT = 0x6C;    // uint32_t count (element count)
                static constexpr uintptr_t PLAYER_LIST = 0x80;             // ChCliPlayer** array
                static constexpr uintptr_t PLAYER_LIST_CAPACITY = 0x88;    // uint32_t capacity (element count)
                static constexpr uintptr_t PLAYER_LIST_COUNT = 0x8C;       // uint32_t count (element count)
                static constexpr uintptr_t LOCAL_PLAYER = 0x98;            // ChCliCharacter* local player
            };

        public:
            ChCliContext(void* ptr) : ForeignClass(ptr) {}

            ChCliCharacter** GetCharacterList() const {
                LOG_MEMORY("ChCliContext", "GetCharacterList", data(), Offsets::CHARACTER_LIST);
                
                ChCliCharacter** characterList = ReadArrayPointer<ChCliCharacter*>(Offsets::CHARACTER_LIST);
                
                LOG_PTR("CharacterList", characterList);
                return characterList;
            }

            uint32_t GetCharacterListCapacity() const {
                LOG_MEMORY("ChCliContext", "GetCharacterListCapacity", data(), Offsets::CHARACTER_LIST_CAPACITY);
                
                uint32_t capacity = ReadMemberFast<uint32_t>(Offsets::CHARACTER_LIST_CAPACITY, 0);
                
                LOG_DEBUG("ChCliContext::GetCharacterListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetCharacterListCount() const {
                LOG_MEMORY("ChCliContext", "GetCharacterListCount", data(), Offsets::CHARACTER_LIST_COUNT);
                
                uint32_t count = ReadMemberFast<uint32_t>(Offsets::CHARACTER_LIST_COUNT, 0);
                
                LOG_DEBUG("ChCliContext::GetCharacterListCount - Count: %u", count);
                return count;
            }

            ChCliPlayer** GetPlayerList() const {
                LOG_MEMORY("ChCliContext", "GetPlayerList", data(), Offsets::PLAYER_LIST);
                
                ChCliPlayer** playerList = ReadArrayPointer<ChCliPlayer*>(Offsets::PLAYER_LIST);
                
                LOG_PTR("PlayerList", playerList);
                return playerList;
            }

            uint32_t GetPlayerListCapacity() const {
                LOG_MEMORY("ChCliContext", "GetPlayerListCapacity", data(), Offsets::PLAYER_LIST_CAPACITY);
                
                uint32_t capacity = ReadMemberFast<uint32_t>(Offsets::PLAYER_LIST_CAPACITY, 0);
                
                LOG_DEBUG("ChCliContext::GetPlayerListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetPlayerListCount() const {
                LOG_MEMORY("ChCliContext", "GetPlayerListCount", data(), Offsets::PLAYER_LIST_COUNT);
                
                uint32_t count = ReadMemberFast<uint32_t>(Offsets::PLAYER_LIST_COUNT, 0);
                
                LOG_DEBUG("ChCliContext::GetPlayerListCount - Count: %u", count);
                return count;
            }

            ChCliCharacter GetLocalPlayer() const {
                LOG_MEMORY("ChCliContext", "GetLocalPlayer", data(), Offsets::LOCAL_PLAYER);
                
                ChCliCharacter result = ReadPointerFast<ChCliCharacter>(Offsets::LOCAL_PLAYER);
                
                LOG_PTR("LocalPlayer", result.data());
                return result;
            }

            SafeAccess::SafeGameArray<ChCliCharacter> GetCharacters() const {
                return SafeAccess::SafeGameArray<ChCliCharacter>(
                    ReadMemberFast<void*>(Offsets::CHARACTER_LIST, nullptr),
                    ReadMemberFast<uint32_t>(Offsets::CHARACTER_LIST_CAPACITY, 0)
                );
            }

            SafeAccess::SafeGameArray<ChCliPlayer> GetPlayers() const {
                return SafeAccess::SafeGameArray<ChCliPlayer>(
                    ReadMemberFast<void*>(Offsets::PLAYER_LIST, nullptr),
                    ReadMemberFast<uint32_t>(Offsets::PLAYER_LIST_CAPACITY, 0)
                );
            }
        };

        /**
         * @brief GdCliContext - Gadget context managing all gadgets/objects
         *
         * Note: CAPACITY/COUNT are element counts (not bytes), represent zone limits not visible entities.
         *       CAPACITY >= COUNT always. Arrays are sparse - use CAPACITY for iteration, validate pointers.
         *
         * Attack target list (walls, destructible objects, etc.):
         * - Internal class: Gw2::Engine::Agent::AgentInl
         * - Entries are AgentInl structures pointing to AgKeyframed with TYPE=11 (GadgetAttackTarget)
         */
        class GdCliContext : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t GADGET_LIST = 0x0030;          // GdCliGadget** array
                static constexpr uintptr_t GADGET_LIST_CAPACITY = 0x0038; // uint32_t capacity (element count)
                static constexpr uintptr_t GADGET_LIST_COUNT = 0x003C;    // uint32_t count (element count)

                static constexpr uintptr_t ATTACK_TARGET_LIST = 0x0010;          // AgentInl** array
                static constexpr uintptr_t ATTACK_TARGET_LIST_CAPACITY = 0x0018; // uint32_t capacity (element count)
                static constexpr uintptr_t ATTACK_TARGET_LIST_COUNT = 0x001C;    // uint32_t count (element count)
            };

        public:
            GdCliContext(void* ptr) : ForeignClass(ptr) {}

            GdCliGadget** GetGadgetList() const {
                LOG_MEMORY("GdCliContext", "GetGadgetList", data(), Offsets::GADGET_LIST);
                
                GdCliGadget** gadgetList = ReadArrayPointer<GdCliGadget*>(Offsets::GADGET_LIST);
                
                LOG_PTR("GadgetList", gadgetList);
                return gadgetList;
            }

            uint32_t GetGadgetListCapacity() const {
                LOG_MEMORY("GdCliContext", "GetGadgetListCapacity", data(), Offsets::GADGET_LIST_CAPACITY);
                
                uint32_t capacity = ReadMemberFast<uint32_t>(Offsets::GADGET_LIST_CAPACITY, 0);
                
                LOG_DEBUG("GdCliContext::GetGadgetListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetGadgetListCount() const {
                LOG_MEMORY("GdCliContext", "GetGadgetListCount", data(), Offsets::GADGET_LIST_COUNT);
                
                uint32_t count = ReadMemberFast<uint32_t>(Offsets::GADGET_LIST_COUNT, 0);
                
                LOG_DEBUG("GdCliContext::GetGadgetListCount - Count: %u", count);
                return count;
            }

            AgentInl** GetAttackTargetList() const {
                LOG_MEMORY("GdCliContext", "GetAttackTargetList", data(), Offsets::ATTACK_TARGET_LIST);
                
                AgentInl** attackTargetList = ReadArrayPointer<AgentInl*>(Offsets::ATTACK_TARGET_LIST);
                
                LOG_PTR("AttackTargetList", attackTargetList);
                return attackTargetList;
            }

            uint32_t GetAttackTargetListCapacity() const {
                LOG_MEMORY("GdCliContext", "GetAttackTargetListCapacity", data(), Offsets::ATTACK_TARGET_LIST_CAPACITY);
                
                uint32_t capacity = ReadMemberFast<uint32_t>(Offsets::ATTACK_TARGET_LIST_CAPACITY, 0);
                
                LOG_DEBUG("GdCliContext::GetAttackTargetListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetAttackTargetListCount() const {
                LOG_MEMORY("GdCliContext", "GetAttackTargetListCount", data(), Offsets::ATTACK_TARGET_LIST_COUNT);
                
                uint32_t count = ReadMemberFast<uint32_t>(Offsets::ATTACK_TARGET_LIST_COUNT, 0);
                
                LOG_DEBUG("GdCliContext::GetAttackTargetListCount - Count: %u", count);
                return count;
            }

            SafeAccess::SafeGameArray<GdCliGadget> GetGadgets() const {
                return SafeAccess::SafeGameArray<GdCliGadget>(
                    ReadMemberFast<void*>(Offsets::GADGET_LIST, nullptr),
                    ReadMemberFast<uint32_t>(Offsets::GADGET_LIST_CAPACITY, 0)
                );
            }

            SafeAccess::SafeGameArray<AgentInl> GetAttackTargets() const {
                return SafeAccess::SafeGameArray<AgentInl>(
                    ReadMemberFast<void*>(Offsets::ATTACK_TARGET_LIST, nullptr),
                    ReadMemberFast<uint32_t>(Offsets::ATTACK_TARGET_LIST_CAPACITY, 0)
                );
            }
        };

        /**
         * @brief ItCliContext - Item context managing all items
         *
         * Note: CAPACITY/COUNT are element counts (not bytes), represent zone limits not visible entities.
         *       CAPACITY >= COUNT always. Arrays are sparse - use CAPACITY for iteration, validate pointers.
         */
        class ItCliContext : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t ITEM_LIST = 0x30;          // ItCliItem** array
                static constexpr uintptr_t ITEM_LIST_CAPACITY = 0x38; // uint32_t capacity (element count)
                static constexpr uintptr_t ITEM_LIST_COUNT = 0x3C;    // uint32_t count (element count)
            };

        public:
            ItCliContext(void* ptr) : ForeignClass(ptr) {}

            ItCliItem** GetItemList() const {
                LOG_MEMORY("ItCliContext", "GetItemList", data(), Offsets::ITEM_LIST);
                
                ItCliItem** itemList = ReadArrayPointer<ItCliItem*>(Offsets::ITEM_LIST);
                
                LOG_PTR("ItemList", itemList);
                return itemList;
            }

            uint32_t GetCapacity() const {
                LOG_MEMORY("ItCliContext", "GetCapacity", data(), Offsets::ITEM_LIST_CAPACITY);
                
                uint32_t capacity = ReadMemberFast<uint32_t>(Offsets::ITEM_LIST_CAPACITY, 0);
                
                LOG_DEBUG("ItCliContext::GetCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetCount() const {
                LOG_MEMORY("ItCliContext", "GetCount", data(), Offsets::ITEM_LIST_COUNT);
                
                uint32_t count = ReadMemberFast<uint32_t>(Offsets::ITEM_LIST_COUNT, 0);
                
                LOG_DEBUG("ItCliContext::GetCount - Count: %u", count);
                return count;
            }

            SafeAccess::SafeGameArray<ItCliItem> GetItems() const {
                return SafeAccess::SafeGameArray<ItCliItem>(
                    ReadMemberFast<void*>(Offsets::ITEM_LIST, nullptr),
                    ReadMemberFast<uint32_t>(Offsets::ITEM_LIST_CAPACITY, 0)
                );
            }
        };

        /**
         * @brief ContextCollection - Root collection containing all context managers
         */
        class ContextCollection : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CH_CLI_CONTEXT = 0x98;   // ChCliContext* character context
                static constexpr uintptr_t GD_CLI_CONTEXT = 0x0138; // GdCliContext* gadget context
                static constexpr uintptr_t IT_CLI_CONTEXT = 0x0178; // ItCliContext* item context
            };

        public:
            ContextCollection(void* ptr) : ForeignClass(ptr) {
                // Debug: Log the ContextCollection base address using proper logging system
                if (ptr) {
                    LOG_DEBUG("ContextCollection base = 0x" + std::to_string(reinterpret_cast<uintptr_t>(ptr)));
                }
            }

            ChCliContext GetChCliContext() const {
                LOG_MEMORY("ContextCollection", "GetChCliContext", data(), Offsets::CH_CLI_CONTEXT);
                
                ChCliContext result = ReadPointerFast<ChCliContext>(Offsets::CH_CLI_CONTEXT);
                
                LOG_PTR("ChCliContext", result.data());
                return result;
            }

            GdCliContext GetGdCliContext() const {
                LOG_MEMORY("ContextCollection", "GetGdCliContext", data(), Offsets::GD_CLI_CONTEXT);
                
                GdCliContext result = ReadPointerFast<GdCliContext>(Offsets::GD_CLI_CONTEXT);
                
                LOG_PTR("GdCliContext", result.data());
                return result;
            }

            ItCliContext GetItCliContext() const {
                LOG_MEMORY("ContextCollection", "GetItCliContext", data(), Offsets::IT_CLI_CONTEXT);
                
                ItCliContext result = ReadPointerFast<ItCliContext>(Offsets::IT_CLI_CONTEXT);
                
                LOG_PTR("ItCliContext", result.data());
                return result;
            }
        };

    } // namespace ReClass
} // namespace kx
