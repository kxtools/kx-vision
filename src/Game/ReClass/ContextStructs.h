#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Core/AppState.h"
#include "../../Utils/ForeignClass.h"
#include "CharacterStructs.h"
#include "GadgetStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Character context manager - handles character and player lists
         */
        class ChCliContext : public ForeignClass {
        public:
            ChCliContext(void* ptr) : ForeignClass(ptr) {}

            ChCliCharacter** GetCharacterList() {
                LOG_MEMORY("ChCliContext", "GetCharacterList", data(), 0x60);
                
                if (!data()) {
                    LOG_ERROR("ChCliContext::GetCharacterList - ChCliContext data is null");
                    return nullptr;
                }
                
                ChCliCharacter** characterList = nullptr;
                if (!Debug::SafeRead<ChCliCharacter**>(data(), 0x60, characterList)) {
                    LOG_ERROR("ChCliContext::GetCharacterList - Failed to read CharacterList pointer at offset 0x60");
                    return nullptr;
                }
                
                LOG_PTR("CharacterList", characterList);
                return characterList;
            }

            uint32_t GetCharacterListCapacity() {
                LOG_MEMORY("ChCliContext", "GetCharacterListCapacity", data(), 0x68);
                
                if (!data()) {
                    LOG_ERROR("ChCliContext::GetCharacterListCapacity - ChCliContext data is null");
                    return 0;
                }
                
                uint32_t capacity = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x68, capacity)) {
                    LOG_ERROR("ChCliContext::GetCharacterListCapacity - Failed to read capacity at offset 0x68");
                    return 0;
                }
                
                LOG_DEBUG("ChCliContext::GetCharacterListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            ChCliPlayer** GetPlayerList() {
                LOG_MEMORY("ChCliContext", "GetPlayerList", data(), 0x80);
                
                if (!data()) {
                    LOG_ERROR("ChCliContext::GetPlayerList - ChCliContext data is null");
                    return nullptr;
                }
                
                ChCliPlayer** playerList = nullptr;
                if (!Debug::SafeRead<ChCliPlayer**>(data(), 0x80, playerList)) {
                    LOG_ERROR("ChCliContext::GetPlayerList - Failed to read PlayerList pointer at offset 0x80");
                    return nullptr;
                }
                
                LOG_PTR("PlayerList", playerList);
                return playerList;
            }

            uint32_t GetPlayerListSize() {
                LOG_MEMORY("ChCliContext", "GetPlayerListSize", data(), 0x88);
                
                if (!data()) {
                    LOG_ERROR("ChCliContext::GetPlayerListSize - ChCliContext data is null");
                    return 0;
                }
                
                uint32_t size = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x88, size)) {
                    LOG_ERROR("ChCliContext::GetPlayerListSize - Failed to read size at offset 0x88");
                    return 0;
                }
                
                LOG_DEBUG("ChCliContext::GetPlayerListSize - Size: %u", size);
                return size;
            }

            ChCliCharacter* GetLocalPlayer() {
                LOG_MEMORY("ChCliContext", "GetLocalPlayer", data(), 0x98);
                
                if (!data()) {
                    LOG_ERROR("ChCliContext::GetLocalPlayer - Context data is null");
                    return nullptr;
                }
                
                ChCliCharacter* result = nullptr;
                if (!Debug::SafeReadWithLogging<ChCliCharacter*>(data(), 0x98, result, "ChCliContext::GetLocalPlayer")) {
                    return nullptr;
                }
                
                LOG_PTR("LocalPlayer", result);
                return result;
            }
        };

        /**
         * @brief Gadget context manager - handles gadget lists
         */
        class GdCliContext : public ForeignClass {
        public:
            GdCliContext(void* ptr) : ForeignClass(ptr) {}

            GdCliGadget** GetGadgetList() {
                LOG_MEMORY("GdCliContext", "GetGadgetList", data(), 0x0030);
                
                if (!data()) {
                    LOG_ERROR("GdCliContext::GetGadgetList - GdCliContext data is null");
                    return nullptr;
                }
                
                GdCliGadget** gadgetList = nullptr;
                if (!Debug::SafeRead<GdCliGadget**>(data(), 0x0030, gadgetList)) {
                    LOG_ERROR("GdCliContext::GetGadgetList - Failed to read GadgetList pointer at offset 0x0030");
                    return nullptr;
                }
                
                LOG_PTR("GadgetList", gadgetList);
                return gadgetList;
            }

            uint32_t GetGadgetListCapacity() {
                LOG_MEMORY("GdCliContext", "GetGadgetListCapacity", data(), 0x0038);
                
                if (!data()) {
                    LOG_ERROR("GdCliContext::GetGadgetListCapacity - GdCliContext data is null");
                    return 0;
                }
                
                uint32_t capacity = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x0038, capacity)) {
                    LOG_ERROR("GdCliContext::GetGadgetListCapacity - Failed to read capacity at offset 0x0038");
                    return 0;
                }
                
                LOG_DEBUG("GdCliContext::GetGadgetListCapacity - Capacity: %u", capacity);
                return capacity;
            }

            uint32_t GetGadgetListCount() {
                LOG_MEMORY("GdCliContext", "GetGadgetListCount", data(), 0x003C);
                
                if (!data()) {
                    LOG_ERROR("GdCliContext::GetGadgetListCount - GdCliContext data is null");
                    return 0;
                }
                
                uint32_t count = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x003C, count)) {
                    LOG_ERROR("GdCliContext::GetGadgetListCount - Failed to read count at offset 0x003C");
                    return 0;
                }
                
                LOG_DEBUG("GdCliContext::GetGadgetListCount - Count: %u", count);
                return count;
            }
        };

        /**
         * @brief Root context collection - entry point for all game context access
         */
        class ContextCollection : public ForeignClass {
        public:
            ContextCollection(void* ptr) : ForeignClass(ptr) {
                // Debug: Log the ContextCollection base address using proper logging system
                if (ptr) {
                    LOG_DEBUG("ContextCollection base = 0x" + std::to_string(reinterpret_cast<uintptr_t>(ptr)));
                }
            }

            ChCliContext GetChCliContext() {
                LOG_MEMORY("ContextCollection", "GetChCliContext", data(), 0x98);
                
                if (!data()) {
                    LOG_ERROR("ContextCollection::GetChCliContext - ContextCollection data is null");
                    return ChCliContext(nullptr);
                }
                
                void* contextPtr = nullptr;
                if (!Debug::SafeReadWithLogging<void*>(data(), 0x98, contextPtr, "ContextCollection::GetChCliContext")) {
                    return ChCliContext(nullptr);
                }
                
                LOG_PTR("ChCliContext", contextPtr);
                return ChCliContext(contextPtr);
            }

            GdCliContext GetGdCliContext() {
                LOG_MEMORY("ContextCollection", "GetGdCliContext", data(), 0x0138);
                
                if (!data()) {
                    LOG_ERROR("ContextCollection::GetGdCliContext - ContextCollection data is null");
                    return GdCliContext(nullptr);
                }
                
                void* gdContextPtr = nullptr;
                if (!Debug::SafeReadWithLogging<void*>(data(), 0x0138, gdContextPtr, "ContextCollection::GetGdCliContext")) {
                    return GdCliContext(nullptr);
                }
                
                LOG_PTR("GdCliContext", gdContextPtr);
                return GdCliContext(gdContextPtr);
            }
        };

    } // namespace ReClass
} // namespace kx