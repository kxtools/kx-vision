#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Memory/SafeForeignClass.h"
#include "../GameEnums.h"
#include "AgentStructs.h"
#include "EquipmentStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief ChCliHealth - Character health management
         */
        class ChCliHealth : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CURRENT = 0x0C;  // float current health
                static constexpr uintptr_t MAX = 0x10;      // float maximum health
                static constexpr uintptr_t HEALTH_REGEN_RATE = 0x14; // float health regeneration rate (0 in combat, often 10% of max HP otherwise)
                static constexpr uintptr_t BARRIER = 0x28;  // float current barrier
            };

        public:
            ChCliHealth(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() const { 
                LOG_MEMORY("ChCliHealth", "GetCurrent", data(), Offsets::CURRENT);
                
                float current = ReadMemberFast<float>(Offsets::CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() const { 
                LOG_MEMORY("ChCliHealth", "GetMax", data(), Offsets::MAX);
                
                float max = ReadMemberFast<float>(Offsets::MAX, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetMax - Max: %.2f", max);
                return max;
            }

            float GetHealthRegenRate() const { 
                LOG_MEMORY("ChCliHealth", "GetHealthRegenRate", data(), Offsets::HEALTH_REGEN_RATE);
                
                float regenRate = ReadMemberFast<float>(Offsets::HEALTH_REGEN_RATE, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetHealthRegenRate - Regen Rate: %.2f", regenRate);
                return regenRate;
            }

            float GetBarrier() const {
                LOG_MEMORY("ChCliHealth", "GetBarrier", data(), Offsets::BARRIER);

                float barrier = ReadMemberFast<float>(Offsets::BARRIER, 0.0f);

                LOG_DEBUG("ChCliHealth::GetBarrier - Barrier: %.2f", barrier);
                return barrier;
            }
        };

        /**
         * @brief ChCliEnergies - Character mount/special energy management
         */
        class ChCliEnergies : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CURRENT = 0x0C;  // float current energy
                static constexpr uintptr_t MAX = 0x10;      // float maximum energy
            };

        public:
            ChCliEnergies(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() const { 
                LOG_MEMORY("ChCliEnergies", "GetCurrent", data(), Offsets::CURRENT);
                
                float current = ReadMemberFast<float>(Offsets::CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliEnergies::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() const { 
                LOG_MEMORY("ChCliEnergies", "GetMax", data(), Offsets::MAX);
                
                float max = ReadMemberFast<float>(Offsets::MAX, 0.0f);
                
                LOG_DEBUG("ChCliEnergies::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief ChCliEndurance - Character dodge/endurance management
         * 
         * Note: A second pool might exist at offsets 0x18/0x20
         */
        class ChCliEndurance : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CURRENT = 0x10;  // float current endurance
                static constexpr uintptr_t MAX = 0x14;      // float maximum endurance
            };

        public:
            ChCliEndurance(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() const { 
                LOG_MEMORY("ChCliEndurance", "GetCurrent", data(), Offsets::CURRENT);
                
                float current = ReadMemberFast<float>(Offsets::CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliEndurance::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() const { 
                LOG_MEMORY("ChCliEndurance", "GetMax", data(), Offsets::MAX);
                
                float max = ReadMemberFast<float>(Offsets::MAX, 0.0f);
                
                LOG_DEBUG("ChCliEndurance::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief ChCliCoreStats - Character core statistics (race, level, profession)
         */
        class ChCliCoreStats : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t RACE = 0x33;          // uint8_t race ID
                static constexpr uintptr_t LEVEL = 0xAC;         // uint32_t actual level
                static constexpr uintptr_t PROFESSION = 0x12C;   // uint32_t profession ID
                static constexpr uintptr_t SCALED_LEVEL = 0x234; // uint32_t scaled/effective level
            };

        public:
            ChCliCoreStats(void* ptr) : SafeForeignClass(ptr) {}
            
            Game::Race GetRace() const { 
                LOG_MEMORY("ChCliCoreStats", "GetRace", data(), Offsets::RACE);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetRace - ChCliCoreStats data is null");
                    return Game::Race::None;
                }

                uint8_t raceValue = ReadMemberFast<uint8_t>(Offsets::RACE, 0);
                Game::Race race = static_cast<Game::Race>(raceValue);
                LOG_DEBUG("ChCliCoreStats::GetRace - Race: %u", static_cast<uint8_t>(race));
                return race;
            }
            
            uint32_t GetLevel() const { 
                LOG_MEMORY("ChCliCoreStats", "GetLevel", data(), Offsets::LEVEL);
                
                uint32_t level = ReadMemberFast<uint32_t>(Offsets::LEVEL, 0);
                
                LOG_DEBUG("ChCliCoreStats::GetLevel - Level: %u", level);
                return level;
            }

            uint32_t GetScaledLevel() const {
                LOG_MEMORY("ChCliCoreStats", "GetScaledLevel", data(), Offsets::SCALED_LEVEL);

                uint32_t scaledLevel = ReadMemberFast<uint32_t>(Offsets::SCALED_LEVEL, 0);

                LOG_DEBUG("ChCliCoreStats::GetScaledLevel - Level: %u", scaledLevel);
                return scaledLevel;
            }

            Game::Profession GetProfession() const { 
                LOG_MEMORY("ChCliCoreStats", "GetProfession", data(), Offsets::PROFESSION);
                
                uint32_t profValue = ReadMemberFast<uint32_t>(Offsets::PROFESSION, 0);
                Game::Profession profession = static_cast<Game::Profession>(profValue);
                
                LOG_DEBUG("ChCliCoreStats::GetProfession - Profession: %u", static_cast<uint32_t>(profession));
                return profession;
            }
        };

        /**
         * @brief ChCliCharacter - Main character structure containing all subsystems
         */
        class ChCliCharacter : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t AGENT = 0x98;            // AgChar* character's agent
                static constexpr uintptr_t ATTITUDE = 0x00C0;       // uint32_t attitude flags
                static constexpr uintptr_t BREAKBAR = 0x00C8;       // CmbtCliBreakBar* breakbar subsystem
                static constexpr uintptr_t RANK_FLAGS = 0x0264;     // uint32_t rank flags (veteran, elite, etc.)
                static constexpr uintptr_t CORE_STATS = 0x0388;     // ChCliCoreStats* stats subsystem
                static constexpr uintptr_t ENDURANCE = 0x03D0;      // ChCliEndurance* dodge/endurance subsystem
                static constexpr uintptr_t ENERGIES = 0x03D8;       // ChCliEnergies* mount/special energy subsystem
                static constexpr uintptr_t FORCE = 0x03E0;          // ChCliForce* force subsystem
                static constexpr uintptr_t HEALTH = 0x03E8;         // ChCliHealth* health subsystem
                static constexpr uintptr_t INVENTORY = 0x3F0;       // ChCliInventory* inventory subsystem
                static constexpr uintptr_t SKILLBAR = 0x0520;       // ChCliSkillbar* skillbar subsystem
            };

        public:
            ChCliCharacter(void* ptr) : SafeForeignClass(ptr) {}

            AgChar GetAgent() const {
                return ReadPointerFast<AgChar>(Offsets::AGENT);
            }

            ChCliHealth GetHealth() const { 
                LOG_MEMORY("ChCliCharacter", "GetHealth", data(), Offsets::HEALTH);
                
                ChCliHealth result = ReadPointerFast<ChCliHealth>(Offsets::HEALTH);
                
                LOG_PTR("Health", result.data());
                return result;
            }

            ChCliEndurance GetEndurance() const { 
                LOG_MEMORY("ChCliCharacter", "GetEndurance", data(), Offsets::ENDURANCE);
                
                ChCliEndurance result = ReadPointerFast<ChCliEndurance>(Offsets::ENDURANCE);
                
                LOG_PTR("Endurance", result.data());
                return result;
            }

            ChCliEnergies GetEnergies() const { 
                LOG_MEMORY("ChCliCharacter", "GetEnergies", data(), Offsets::ENERGIES);
                
                ChCliEnergies result = ReadPointerFast<ChCliEnergies>(Offsets::ENERGIES);
                
                LOG_PTR("Energies", result.data());
                return result;
            }

            ChCliCoreStats GetCoreStats() const { 
                LOG_MEMORY("ChCliCharacter", "GetCoreStats", data(), Offsets::CORE_STATS);
                
                ChCliCoreStats result = ReadPointerFast<ChCliCoreStats>(Offsets::CORE_STATS);
                
                LOG_PTR("CoreStats", result.data());
                return result;
            }

            Game::Attitude GetAttitude() const {
                LOG_MEMORY("ChCliCharacter", "GetAttitude", data(), Offsets::ATTITUDE);

                uint32_t attitudeValue = ReadMemberFast<uint32_t>(Offsets::ATTITUDE, 1);
                Game::Attitude attitude = static_cast<Game::Attitude>(attitudeValue);
                
                LOG_DEBUG("ChCliCharacter::GetAttitude - Attitude: %u", static_cast<uint32_t>(attitude));
                return attitude;
            }

            Game::CharacterRank GetRank() const {
                LOG_MEMORY("ChCliCharacter", "GetRank", data(), Offsets::RANK_FLAGS);

                uint32_t flags = ReadMemberFast<uint32_t>(Offsets::RANK_FLAGS, 0);

                // Check from highest rank to lowest
                if ((flags & static_cast<uint32_t>(Game::CharacterRankFlags::Legendary)) != 0)
                    return Game::CharacterRank::Legendary;
                if ((flags & static_cast<uint32_t>(Game::CharacterRankFlags::Champion)) != 0)
                    return Game::CharacterRank::Champion;
                if ((flags & static_cast<uint32_t>(Game::CharacterRankFlags::Elite)) != 0)
                    return Game::CharacterRank::Elite;
                if ((flags & static_cast<uint32_t>(Game::CharacterRankFlags::Veteran)) != 0)
                    return Game::CharacterRank::Veteran;
                if ((flags & static_cast<uint32_t>(Game::CharacterRankFlags::Ambient)) != 0)
                    return Game::CharacterRank::Ambient;

                return Game::CharacterRank::Normal;
            }

            ChCliInventory GetInventory() const {
                return ReadPointerFast<ChCliInventory>(Offsets::INVENTORY);
            }
        };

        /**
         * @brief ChCliPlayer - Player wrapper containing character and name
         */
        class ChCliPlayer : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CHARACTER_PTR = 0x18;  // ChCliCharacter* player's character
                static constexpr uintptr_t NAME_PTR = 0x68;       // wchar_t* player name string
            };

        public:
            ChCliPlayer(void* ptr) : SafeForeignClass(ptr) {}
            
            ChCliCharacter GetCharacter() const { 
                return ReadPointerFast<ChCliCharacter>(Offsets::CHARACTER_PTR);
            }
            
            const wchar_t* GetName() const { 
                return ReadMemberFast<const wchar_t*>(Offsets::NAME_PTR, nullptr);
            }
        };

    } // namespace ReClass
} // namespace kx
