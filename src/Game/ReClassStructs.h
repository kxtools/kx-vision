#pragma once

#include "../Utils/DebugLogger.h"
#include "../Core/AppState.h"  // For IsDebugLoggingEnabled

#include "../Utils/ForeignClass.h"
#include "GameStructs.h" // Re-use Coordinates3D
#include "GameEnums.h"   // Include the new enums

namespace kx {
    namespace ReClass {

	    // Forward declarations
        class ChCliCharacter;
        class AgChar;
        class ChCliHealth;
        class ChCliCoreStats;
        class ChCliPlayer;
        class ChCliEnergies;
        class GdCliGadget;

        // --- Safe Wrappers for Game Structures ---

        class CoChar : public ForeignClass {
        public:
            CoChar(void* ptr) : ForeignClass(ptr) {}

            Coordinates3D GetVisualPosition() {
                // Only log memory access for successful reads to reduce spam
                if (!data()) {
                    return { 0, 0, 0 };
                }
                
                Coordinates3D result;
                if (!Debug::SafeRead<Coordinates3D>(data(), 0x30, result)) {
                    // Reduce log spam for position failures
                    return { 0, 0, 0 };
                }
                
                return result;
            }
        };

        class AgChar : public ForeignClass {
        public:
            AgChar(void* ptr) : ForeignClass(ptr) {}

            CoChar GetCoChar() {
                LOG_MEMORY("AgChar", "GetCoChar", data(), 0x50);
                
                if (!data()) {
                    LOG_ERROR("AgChar::GetCoChar - AgChar data is null");
                    return CoChar(nullptr);
                }
                
                void* coCharPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x50, coCharPtr)) {
                    LOG_ERROR("AgChar::GetCoChar - Failed to read CoChar pointer at offset 0x50");
                    return CoChar(nullptr);
                }
                
                LOG_PTR("CoChar", coCharPtr);
                return CoChar(coCharPtr);
            }

            uint32_t GetType() {
                LOG_MEMORY("AgChar", "GetType", data(), 0x08);
                
                if (!data()) {
                    LOG_ERROR("AgChar::GetType - AgChar data is null");
                    return 0;
                }
                
                uint32_t type = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x08, type)) {
                    LOG_ERROR("AgChar::GetType - Failed to read type at offset 0x08");
                    return 0;
                }
                
                LOG_INFO("AgChar::GetType - Type: %u", type);
                return type;
            }
        };

        class ChCliEnergies : public ForeignClass {
        public:
            ChCliEnergies(void* ptr) : ForeignClass(ptr) {}
            
            float GetCurrent() { 
                LOG_MEMORY("ChCliEnergies", "GetCurrent", data(), 0x0C);
                
                if (!data()) {
                    LOG_ERROR("ChCliEnergies::GetCurrent - ChCliEnergies data is null");
                    return 0.0f;
                }
                
                float current = 0.0f;
                if (!Debug::SafeRead<float>(data(), 0x0C, current)) {
                    LOG_ERROR("ChCliEnergies::GetCurrent - Failed to read current energy at offset 0x0C");
                    return 0.0f;
                }
                
                LOG_INFO("ChCliEnergies::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() { 
                LOG_MEMORY("ChCliEnergies", "GetMax", data(), 0x10);
                
                if (!data()) {
                    LOG_ERROR("ChCliEnergies::GetMax - ChCliEnergies data is null");
                    return 0.0f;
                }
                
                float max = 0.0f;
                if (!Debug::SafeRead<float>(data(), 0x10, max)) {
                    LOG_ERROR("ChCliEnergies::GetMax - Failed to read max energy at offset 0x10");
                    return 0.0f;
                }
                
                LOG_INFO("ChCliEnergies::GetMax - Max: %.2f", max);
                return max;
            }
        };

        class ChCliHealth : public ForeignClass {
        public:
            ChCliHealth(void* ptr) : ForeignClass(ptr) {}
            
            float GetCurrent() { 
                LOG_MEMORY("ChCliHealth", "GetCurrent", data(), 0x0C);
                
                if (!data()) {
                    LOG_ERROR("ChCliHealth::GetCurrent - ChCliHealth data is null");
                    return 0.0f;
                }
                
                float current = 0.0f;
                if (!Debug::SafeRead<float>(data(), 0x0C, current)) {
                    LOG_ERROR("ChCliHealth::GetCurrent - Failed to read current health at offset 0x0C");
                    return 0.0f;
                }
                
                LOG_INFO("ChCliHealth::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() { 
                LOG_MEMORY("ChCliHealth", "GetMax", data(), 0x10);
                
                if (!data()) {
                    LOG_ERROR("ChCliHealth::GetMax - ChCliHealth data is null");
                    return 0.0f;
                }
                
                float max = 0.0f;
                if (!Debug::SafeRead<float>(data(), 0x10, max)) {
                    LOG_ERROR("ChCliHealth::GetMax - Failed to read max health at offset 0x10");
                    return 0.0f;
                }
                
                LOG_INFO("ChCliHealth::GetMax - Max: %.2f", max);
                return max;
            }
        };

        class ChCliCoreStats : public ForeignClass {
        public:
            ChCliCoreStats(void* ptr) : ForeignClass(ptr) {}
            
            Game::Race GetRace() { 
                LOG_MEMORY("ChCliCoreStats", "GetRace", data(), 0x33);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetRace - ChCliCoreStats data is null");
                    return Game::Race::None;
                }
                
                uint8_t raceValue = 0;
                if (!Debug::SafeRead<uint8_t>(data(), 0x33, raceValue)) {
                    LOG_ERROR("ChCliCoreStats::GetRace - Failed to read race at offset 0x33");
                    return Game::Race::None;
                }
                
                Game::Race race = static_cast<Game::Race>(raceValue);
                LOG_INFO("ChCliCoreStats::GetRace - Race: %u", static_cast<uint8_t>(race));
                return race;
            }
            
            uint32_t GetLevel() { 
                LOG_MEMORY("ChCliCoreStats", "GetLevel", data(), 0xAC);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetLevel - ChCliCoreStats data is null");
                    return 0;
                }
                
                uint32_t level = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0xAC, level)) {
                    LOG_ERROR("ChCliCoreStats::GetLevel - Failed to read level at offset 0xAC");
                    return 0;
                }
                
                LOG_INFO("ChCliCoreStats::GetLevel - Level: %u", level);
                return level;
            }
            
            Game::Profession GetProfession() { 
                LOG_MEMORY("ChCliCoreStats", "GetProfession", data(), 0x12C);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetProfession - ChCliCoreStats data is null");
                    return Game::Profession::None;
                }
                
                uint32_t profValue = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x12C, profValue)) {
                    LOG_ERROR("ChCliCoreStats::GetProfession - Failed to read profession at offset 0x12C");
                    return Game::Profession::None;
                }
                
                Game::Profession profession = static_cast<Game::Profession>(profValue);
                LOG_INFO("ChCliCoreStats::GetProfession - Profession: %u", static_cast<uint32_t>(profession));
                return profession;
            }
        };

        class ChCliCharacter : public ForeignClass {
        public:
            ChCliCharacter(void* ptr) : ForeignClass(ptr) {}

            AgChar GetAgent() {
                if (!data()) {
                    return AgChar(nullptr);
                }
                
                void* agentPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x98, agentPtr)) {
                    return AgChar(nullptr);
                }
                
                return AgChar(agentPtr);
            }

            ChCliHealth GetHealth() { 
                LOG_MEMORY("ChCliCharacter", "GetHealth", data(), 0x03E8);
                
                if (!data()) {
                    LOG_ERROR("ChCliCharacter::GetHealth - ChCliCharacter data is null");
                    return ChCliHealth(nullptr);
                }
                
                void* healthPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x03E8, healthPtr)) {
                    LOG_ERROR("ChCliCharacter::GetHealth - Failed to read Health pointer at offset 0x03E8");
                    return ChCliHealth(nullptr);
                }
                
                LOG_PTR("Health", healthPtr);
                return ChCliHealth(healthPtr);
            }

            ChCliCoreStats GetCoreStats() { 
                LOG_MEMORY("ChCliCharacter", "GetCoreStats", data(), 0x0388);
                
                if (!data()) {
                    LOG_ERROR("ChCliCharacter::GetCoreStats - ChCliCharacter data is null");
                    return ChCliCoreStats(nullptr);
                }
                
                void* coreStatsPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x0388, coreStatsPtr)) {
                    LOG_ERROR("ChCliCharacter::GetCoreStats - Failed to read CoreStats pointer at offset 0x0388");
                    return ChCliCoreStats(nullptr);
                }
                
                LOG_PTR("CoreStats", coreStatsPtr);
                return ChCliCoreStats(coreStatsPtr);
            }

            Game::Attitude GetAttitude() { 
                if (!data()) {
                    return Game::Attitude::Neutral;
                }
                
                uint32_t attitudeValue = 1; // Default to Neutral
                if (!Debug::SafeRead<uint32_t>(data(), 0x00C0, attitudeValue)) {
                    return Game::Attitude::Neutral;
                }
                
                Game::Attitude attitude = static_cast<Game::Attitude>(attitudeValue);
                return attitude;
            }

            ChCliEnergies GetEnergies() { 
                LOG_MEMORY("ChCliCharacter", "GetEnergies", data(), 0x03D8);
                
                if (!data()) {
                    LOG_ERROR("ChCliCharacter::GetEnergies - ChCliCharacter data is null");
                    return ChCliEnergies(nullptr);
                }
                
                void* energiesPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x03D8, energiesPtr)) {
                    LOG_ERROR("ChCliCharacter::GetEnergies - Failed to read Energies pointer at offset 0x03D8");
                    return ChCliEnergies(nullptr);
                }
                
                LOG_PTR("Energies", energiesPtr);
                return ChCliEnergies(energiesPtr);
            }
        };

        class ChCliPlayer : public ForeignClass {
        public:
            ChCliPlayer(void* ptr) : ForeignClass(ptr) {}
            
            ChCliCharacter GetCharacter() { 
                if (!data()) {
                    return ChCliCharacter(nullptr);
                }
                
                void* characterPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x18, characterPtr)) {
                    return ChCliCharacter(nullptr);
                }
                
                return ChCliCharacter(characterPtr);
            }
            
            const wchar_t* GetName() { 
                if (!data()) {
                    return nullptr;
                }
                
                wchar_t* namePtr = nullptr;
                if (!Debug::SafeRead<wchar_t*>(data(), 0x68, namePtr)) {
                    return nullptr;
                }
                
                return namePtr;
            }
        };

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
                
                LOG_INFO("ChCliContext::GetCharacterListCapacity - Capacity: %u", capacity);
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
                
                LOG_INFO("ChCliContext::GetPlayerListSize - Size: %u", size);
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
                
                LOG_INFO("GdCliContext::GetGadgetListCapacity - Capacity: %u", capacity);
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
                
                LOG_INFO("GdCliContext::GetGadgetListCount - Count: %u", count);
                return count;
            }
        };

        class CoKeyFramed : public ForeignClass {
        public:
            CoKeyFramed(void* ptr) : ForeignClass(ptr) {}
            
            Coordinates3D GetPosition() {
                LOG_MEMORY("CoKeyFramed", "GetPosition", data(), 0x0030);
                
                if (!data()) {
                    LOG_ERROR("CoKeyFramed::GetPosition - CoKeyFramed data is null");
                    return Coordinates3D{ 0,0,0 };
                }
                
                Coordinates3D position;
                if (!Debug::SafeRead<Coordinates3D>(data(), 0x0030, position)) {
                    LOG_ERROR("CoKeyFramed::GetPosition - Failed to read position at offset 0x0030");
                    return Coordinates3D{ 0,0,0 };
                }
                
                LOG_INFO("CoKeyFramed::GetPosition - Position: (%.2f, %.2f, %.2f)", 
                         position.x, position.y, position.z);
                return position;
            }
        };

        class AgKeyFramed : public ForeignClass {
        public:
            AgKeyFramed(void* ptr) : ForeignClass(ptr) {}
            
            CoKeyFramed GetCoKeyFramed() {
                LOG_MEMORY("AgKeyFramed", "GetCoKeyFramed", data(), 0x0050);
                
                if (!data()) {
                    LOG_ERROR("AgKeyFramed::GetCoKeyFramed - AgKeyFramed data is null");
                    return CoKeyFramed(nullptr);
                }
                
                void* coKeyFramedPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x0050, coKeyFramedPtr)) {
                    LOG_ERROR("AgKeyFramed::GetCoKeyFramed - Failed to read CoKeyFramed pointer at offset 0x0050");
                    return CoKeyFramed(nullptr);
                }
                
                LOG_PTR("CoKeyFramed", coKeyFramedPtr);
                return CoKeyFramed(coKeyFramedPtr);
            }
        };

        class GdCliGadget : public ForeignClass {
        public:
            GdCliGadget(void* ptr) : ForeignClass(ptr) {}

            Game::GadgetType GetGadgetType() {
                LOG_MEMORY("GdCliGadget", "GetGadgetType", data(), 0x0200);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::GetGadgetType - GdCliGadget data is null");
                    return Game::GadgetType::None;
                }
                
                uint32_t typeValue = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x0200, typeValue)) {
                    LOG_ERROR("GdCliGadget::GetGadgetType - Failed to read gadget type at offset 0x0200");
                    return Game::GadgetType::None;
                }
                
                Game::GadgetType gadgetType = static_cast<Game::GadgetType>(typeValue);
                LOG_INFO("GdCliGadget::GetGadgetType - Type: %u", static_cast<uint32_t>(gadgetType));
                return gadgetType;
            }

            // This is the key for filtering depleted resource nodes.
            // The flag 0x2 appears to mean "is active/gatherable".
            bool IsGatherable() {
                LOG_MEMORY("GdCliGadget", "IsGatherable", data(), 0x04E8);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::IsGatherable - GdCliGadget data is null");
                    return false;
                }
                
                uint32_t flags = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x04E8, flags)) {
                    LOG_ERROR("GdCliGadget::IsGatherable - Failed to read flags at offset 0x04E8");
                    return false;
                }
                
                bool gatherable = (flags & 0x2) != 0;
                LOG_INFO("GdCliGadget::IsGatherable - Flags: 0x%X, Gatherable: %s", flags, gatherable ? "true" : "false");
                return gatherable;
            }

            AgKeyFramed GetAgKeyFramed() {
                LOG_MEMORY("GdCliGadget", "GetAgKeyFramed", data(), 0x0038);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::GetAgKeyFramed - GdCliGadget data is null");
                    return AgKeyFramed(nullptr);
                }
                
                void* agKeyFramedPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x0038, agKeyFramedPtr)) {
                    LOG_ERROR("GdCliGadget::GetAgKeyFramed - Failed to read AgKeyFramed pointer at offset 0x0038");
                    return AgKeyFramed(nullptr);
                }
                
                LOG_PTR("AgKeyFramed", agKeyFramedPtr);
                return AgKeyFramed(agKeyFramedPtr);
            }
        };

        class ContextCollection : public ForeignClass {
        public:
            ContextCollection(void* ptr) : ForeignClass(ptr) {
                // Debug: Log the ContextCollection base address
                if (ptr && kx::IsDebugLoggingEnabled()) {
                    printf("DEBUG: ContextCollection base = 0x%p\n", ptr);
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
