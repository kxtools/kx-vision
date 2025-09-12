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
        class ChCliHealth : public kx::SafeForeignClass {
        public:
            ChCliHealth(void* ptr) : kx::SafeForeignClass(ptr) {}
            
            float GetCurrent() { 
                LOG_MEMORY("ChCliHealth", "GetCurrent", data(), Offsets::CH_CLI_HEALTH_CURRENT);
                
                float current = ReadMember<float>(Offsets::CH_CLI_HEALTH_CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() { 
                LOG_MEMORY("ChCliHealth", "GetMax", data(), Offsets::CH_CLI_HEALTH_MAX);
                
                float max = ReadMember<float>(Offsets::CH_CLI_HEALTH_MAX, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief Character energy management wrapper
         */
        class ChCliEnergies : public kx::SafeForeignClass {
        public:
            ChCliEnergies(void* ptr) : kx::SafeForeignClass(ptr) {}
            
            float GetCurrent() { 
                LOG_MEMORY("ChCliEnergies", "GetCurrent", data(), Offsets::CH_CLI_ENERGIES_CURRENT);
                
                float current = ReadMember<float>(Offsets::CH_CLI_ENERGIES_CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliEnergies::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() { 
                LOG_MEMORY("ChCliEnergies", "GetMax", data(), Offsets::CH_CLI_ENERGIES_MAX);
                
                float max = ReadMember<float>(Offsets::CH_CLI_ENERGIES_MAX, 0.0f);
                
                LOG_DEBUG("ChCliEnergies::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief Character core statistics wrapper
         */
        class ChCliCoreStats : public kx::SafeForeignClass {
        public:
            ChCliCoreStats(void* ptr) : kx::SafeForeignClass(ptr) {}
            
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
                
                uint32_t level = ReadMember<uint32_t>(Offsets::CH_CLI_CORE_STATS_LEVEL, 0);
                
                LOG_DEBUG("ChCliCoreStats::GetLevel - Level: %u", level);
                return level;
            }
            
            Game::Profession GetProfession() { 
                LOG_MEMORY("ChCliCoreStats", "GetProfession", data(), Offsets::CH_CLI_CORE_STATS_PROFESSION);
                
                uint32_t profValue = ReadMember<uint32_t>(Offsets::CH_CLI_CORE_STATS_PROFESSION, 0);
                Game::Profession profession = static_cast<Game::Profession>(profValue);
                
                LOG_DEBUG("ChCliCoreStats::GetProfession - Profession: %u", static_cast<uint32_t>(profession));
                return profession;
            }
        };

        /**
         * @brief Main character wrapper with access to all character subsystems
         */
        class ChCliCharacter : public kx::SafeForeignClass {
        public:
            ChCliCharacter(void* ptr) : kx::SafeForeignClass(ptr) {}

            AgChar GetAgent() {
                return ReadPointer<AgChar>(Offsets::CH_CLI_CHARACTER_AGENT);
            }

            ChCliHealth GetHealth() { 
                LOG_MEMORY("ChCliCharacter", "GetHealth", data(), Offsets::CH_CLI_CHARACTER_HEALTH);
                
                ChCliHealth result = ReadPointer<ChCliHealth>(Offsets::CH_CLI_CHARACTER_HEALTH);
                
                LOG_PTR("Health", result.data());
                return result;
            }

            ChCliEnergies GetEnergies() { 
                LOG_MEMORY("ChCliCharacter", "GetEnergies", data(), Offsets::CH_CLI_CHARACTER_ENERGIES);
                
                ChCliEnergies result = ReadPointer<ChCliEnergies>(Offsets::CH_CLI_CHARACTER_ENERGIES);
                
                LOG_PTR("Energies", result.data());
                return result;
            }

            ChCliCoreStats GetCoreStats() { 
                LOG_MEMORY("ChCliCharacter", "GetCoreStats", data(), Offsets::CH_CLI_CHARACTER_CORE_STATS);
                
                ChCliCoreStats result = ReadPointer<ChCliCoreStats>(Offsets::CH_CLI_CHARACTER_CORE_STATS);
                
                LOG_PTR("CoreStats", result.data());
                return result;
            }

            Game::Attitude GetAttitude() { 
                LOG_MEMORY("ChCliCharacter", "GetAttitude", data(), Offsets::CH_CLI_CHARACTER_ATTITUDE);
                
                uint32_t attitudeValue = ReadMember<uint32_t>(Offsets::CH_CLI_CHARACTER_ATTITUDE, 0);
                Game::Attitude attitude = static_cast<Game::Attitude>(attitudeValue);
                
                LOG_DEBUG("ChCliCharacter::GetAttitude - Attitude: %u", static_cast<uint32_t>(attitude));
                return attitude;
            }
        };

        /**
         * @brief Player wrapper that contains character data and player name
         */
        class ChCliPlayer : public kx::SafeForeignClass {
        public:
            ChCliPlayer(void* ptr) : kx::SafeForeignClass(ptr) {}
            
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