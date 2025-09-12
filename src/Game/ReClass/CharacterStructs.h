#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Core/AppState.h"
#include "../../Utils/SafeForeignClass.h"
#include "../Coordinates.h"
#include "../GameEnums.h"
#include "../offsets.h"
#include "AgentStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Character health management wrapper
         */
        class ChCliHealth : public SafeForeignClass {
        public:
            ChCliHealth(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() { 
                LOG_MEMORY("ChCliHealth", "GetCurrent", data(), Offsets::CH_CLI_HEALTH_CURRENT);
                
                if (!data()) {
                    LOG_ERROR("ChCliHealth::GetCurrent - ChCliHealth data is null");
                    return 0.0f;
                }
                
                float current = 0.0f;
                if (!Debug::SafeRead<float>(data(), Offsets::CH_CLI_HEALTH_CURRENT, current)) {
                    LOG_ERROR("ChCliHealth::GetCurrent - Failed to read current health at offset 0x0C");
                    return 0.0f;
                }
                
                LOG_DEBUG("ChCliHealth::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() { 
                LOG_MEMORY("ChCliHealth", "GetMax", data(), Offsets::CH_CLI_HEALTH_MAX);
                
                if (!data()) {
                    LOG_ERROR("ChCliHealth::GetMax - ChCliHealth data is null");
                    return 0.0f;
                }
                
                float max = 0.0f;
                if (!Debug::SafeRead<float>(data(), Offsets::CH_CLI_HEALTH_MAX, max)) {
                    LOG_ERROR("ChCliHealth::GetMax - Failed to read max health at offset 0x10");
                    return 0.0f;
                }
                
                LOG_DEBUG("ChCliHealth::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief Character energy management wrapper
         */
        class ChCliEnergies : public SafeForeignClass {
        public:
            ChCliEnergies(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() { 
                LOG_MEMORY("ChCliEnergies", "GetCurrent", data(), Offsets::CH_CLI_ENERGIES_CURRENT);
                
                if (!data()) {
                    LOG_ERROR("ChCliEnergies::GetCurrent - ChCliEnergies data is null");
                    return 0.0f;
                }
                
                float current = 0.0f;
                if (!Debug::SafeRead<float>(data(), Offsets::CH_CLI_ENERGIES_CURRENT, current)) {
                    LOG_ERROR("ChCliEnergies::GetCurrent - Failed to read current energy at offset 0x0C");
                    return 0.0f;
                }
                
                LOG_DEBUG("ChCliEnergies::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() { 
                LOG_MEMORY("ChCliEnergies", "GetMax", data(), Offsets::CH_CLI_ENERGIES_MAX);
                
                if (!data()) {
                    LOG_ERROR("ChCliEnergies::GetMax - ChCliEnergies data is null");
                    return 0.0f;
                }
                
                float max = 0.0f;
                if (!Debug::SafeRead<float>(data(), Offsets::CH_CLI_ENERGIES_MAX, max)) {
                    LOG_ERROR("ChCliEnergies::GetMax - Failed to read max energy at offset 0x10");
                    return 0.0f;
                }
                
                LOG_DEBUG("ChCliEnergies::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief Character core statistics wrapper
         */
        class ChCliCoreStats : public SafeForeignClass {
        public:
            ChCliCoreStats(void* ptr) : SafeForeignClass(ptr) {}
            
            Game::Race GetRace() { 
                LOG_MEMORY("ChCliCoreStats", "GetRace", data(), Offsets::CH_CLI_CORE_STATS_RACE);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetRace - ChCliCoreStats data is null");
                    return Game::Race::None;
                }
                
                uint8_t raceValue = 0;
                if (!Debug::SafeRead<uint8_t>(data(), Offsets::CH_CLI_CORE_STATS_RACE, raceValue)) {
                    LOG_ERROR("ChCliCoreStats::GetRace - Failed to read race at offset 0x33");
                    return Game::Race::None;
                }
                
                Game::Race race = static_cast<Game::Race>(raceValue);
                LOG_DEBUG("ChCliCoreStats::GetRace - Race: %u", static_cast<uint8_t>(race));
                return race;
            }
            
            uint32_t GetLevel() { 
                LOG_MEMORY("ChCliCoreStats", "GetLevel", data(), Offsets::CH_CLI_CORE_STATS_LEVEL);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetLevel - ChCliCoreStats data is null");
                    return 0;
                }
                
                uint32_t level = 0;
                if (!Debug::SafeRead<uint32_t>(data(), Offsets::CH_CLI_CORE_STATS_LEVEL, level)) {
                    LOG_ERROR("ChCliCoreStats::GetLevel - Failed to read level at offset 0xAC");
                    return 0;
                }
                
                LOG_DEBUG("ChCliCoreStats::GetLevel - Level: %u", level);
                return level;
            }
            
            Game::Profession GetProfession() { 
                LOG_MEMORY("ChCliCoreStats", "GetProfession", data(), Offsets::CH_CLI_CORE_STATS_PROFESSION);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetProfession - ChCliCoreStats data is null");
                    return Game::Profession::None;
                }
                
                uint32_t profValue = 0;
                if (!Debug::SafeRead<uint32_t>(data(), Offsets::CH_CLI_CORE_STATS_PROFESSION, profValue)) {
                    LOG_ERROR("ChCliCoreStats::GetProfession - Failed to read profession at offset 0x12C");
                    return Game::Profession::None;
                }
                
                Game::Profession profession = static_cast<Game::Profession>(profValue);
                LOG_DEBUG("ChCliCoreStats::GetProfession - Profession: %u", static_cast<uint32_t>(profession));
                return profession;
            }
        };

        /**
         * @brief Main character wrapper with access to all character subsystems
         */
        class ChCliCharacter : public SafeForeignClass {
        public:
            ChCliCharacter(void* ptr) : SafeForeignClass(ptr) {}

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

        /**
         * @brief Player wrapper that contains character data and player name
         */
        class ChCliPlayer : public SafeForeignClass {
        public:
            ChCliPlayer(void* ptr) : SafeForeignClass(ptr) {}
            
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

    } // namespace ReClass
} // namespace kx