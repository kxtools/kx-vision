#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Utils/SafeForeignClass.h"
#include "../GameEnums.h"
#include "../offsets.h"
#include "AgentStructs.h"
#include "EquipmentStructs.h"
#include "SkillStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Character health management wrapper
         */
        class ChCliHealth : public SafeForeignClass {
        public:
            ChCliHealth(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() const { 
                LOG_MEMORY("ChCliHealth", "GetCurrent", data(), Offsets::ChCliHealth::CURRENT);
                
                float current = ReadMember<float>(Offsets::ChCliHealth::CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() const { 
                LOG_MEMORY("ChCliHealth", "GetMax", data(), Offsets::ChCliHealth::MAX);
                
                float max = ReadMember<float>(Offsets::ChCliHealth::MAX, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetMax - Max: %.2f", max);
                return max;
            }

            float GetHealthRegenRate() const { 
                LOG_MEMORY("ChCliHealth", "GetHealthRegenRate", data(), Offsets::ChCliHealth::HEALTH_REGEN_RATE);
                
                float regenRate = ReadMember<float>(Offsets::ChCliHealth::HEALTH_REGEN_RATE, 0.0f);
                
                LOG_DEBUG("ChCliHealth::GetHealthRegenRate - Regen Rate: %.2f", regenRate);
                return regenRate;
            }

            float GetBarrier() const {
                LOG_MEMORY("ChCliHealth", "GetBarrier", data(), Offsets::ChCliHealth::BARRIER);

                float barrier = ReadMember<float>(Offsets::ChCliHealth::BARRIER, 0.0f);

                LOG_DEBUG("ChCliHealth::GetBarrier - Barrier: %.2f", barrier);
                return barrier;
            }
        };

        /**
         * @brief Character mount/special energy management wrapper
         */
        class ChCliEnergies : public SafeForeignClass {
        public:
            ChCliEnergies(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() const { 
                LOG_MEMORY("ChCliEnergies", "GetCurrent", data(), Offsets::ChCliEnergies::CURRENT);
                
                float current = ReadMember<float>(Offsets::ChCliEnergies::CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliEnergies::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() const { 
                LOG_MEMORY("ChCliEnergies", "GetMax", data(), Offsets::ChCliEnergies::MAX);
                
                float max = ReadMember<float>(Offsets::ChCliEnergies::MAX, 0.0f);
                
                LOG_DEBUG("ChCliEnergies::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief Character dodge/endurance management wrapper
         */
        class ChCliEndurance : public SafeForeignClass {
        public:
            ChCliEndurance(void* ptr) : SafeForeignClass(ptr) {}
            
            float GetCurrent() const { 
                LOG_MEMORY("ChCliEndurance", "GetCurrent", data(), Offsets::ChCliEndurance::CURRENT);
                
                float current = ReadMember<float>(Offsets::ChCliEndurance::CURRENT, 0.0f);
                
                LOG_DEBUG("ChCliEndurance::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() const { 
                LOG_MEMORY("ChCliEndurance", "GetMax", data(), Offsets::ChCliEndurance::MAX);
                
                float max = ReadMember<float>(Offsets::ChCliEndurance::MAX, 0.0f);
                
                LOG_DEBUG("ChCliEndurance::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief Character skillbar management wrapper
         */
        class ChCliSkillbar : public SafeForeignClass {
        public:
            ChCliSkillbar(void* ptr) : SafeForeignClass(ptr) {}

            CharSkillbar GetCharSkillbar() const {
                LOG_MEMORY("ChCliSkillbar", "GetCharSkillbar", data(), Offsets::ChCliSkillbar::CHAR_SKILLBAR);
                CharSkillbar result = ReadPointer<CharSkillbar>(Offsets::ChCliSkillbar::CHAR_SKILLBAR);
                LOG_PTR("CharSkillbar", result.data());
                return result;
            }
        };

        /**
         * @brief Character core statistics wrapper
         */
        class ChCliCoreStats : public SafeForeignClass {
        public:
            ChCliCoreStats(void* ptr) : SafeForeignClass(ptr) {}
            
            Game::Race GetRace() const { 
                LOG_MEMORY("ChCliCoreStats", "GetRace", data(), Offsets::ChCliCoreStats::RACE);
                
                if (!data()) {
                    LOG_ERROR("ChCliCoreStats::GetRace - ChCliCoreStats data is null");
                    return Game::Race::None;
                }
                
                uint8_t raceValue = 0;
                if (!kx::Debug::SafeRead<uint8_t>(data(), Offsets::ChCliCoreStats::RACE, raceValue)) {
                    LOG_ERROR("ChCliCoreStats::GetRace - Failed to read race at offset 0x33");
                    return Game::Race::None;
                }
                
                Game::Race race = static_cast<Game::Race>(raceValue);
                LOG_DEBUG("ChCliCoreStats::GetRace - Race: %u", static_cast<uint8_t>(race));
                return race;
            }
            
            uint32_t GetLevel() const { 
                LOG_MEMORY("ChCliCoreStats", "GetLevel", data(), Offsets::ChCliCoreStats::LEVEL);
                
                uint32_t level = ReadMember<uint32_t>(Offsets::ChCliCoreStats::LEVEL, 0);
                
                LOG_DEBUG("ChCliCoreStats::GetLevel - Level: %u", level);
                return level;
            }

            uint32_t GetScaledLevel() const {
                LOG_MEMORY("ChCliCoreStats", "GetScaledLevel", data(), Offsets::ChCliCoreStats::SCALED_LEVEL);

                uint32_t scaledLevel = ReadMember<uint32_t>(Offsets::ChCliCoreStats::SCALED_LEVEL, 0);

                LOG_DEBUG("ChCliCoreStats::GetScaledLevel - Level: %u", scaledLevel);
                return scaledLevel;
            }

            Game::Profession GetProfession() const { 
                LOG_MEMORY("ChCliCoreStats", "GetProfession", data(), Offsets::ChCliCoreStats::PROFESSION);
                
                uint32_t profValue = ReadMember<uint32_t>(Offsets::ChCliCoreStats::PROFESSION, 0);
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

            AgChar GetAgent() const {
                return ReadPointer<AgChar>(Offsets::ChCliCharacter::AGENT);
            }

            ChCliHealth GetHealth() const { 
                LOG_MEMORY("ChCliCharacter", "GetHealth", data(), Offsets::ChCliCharacter::HEALTH);
                
                ChCliHealth result = ReadPointer<ChCliHealth>(Offsets::ChCliCharacter::HEALTH);
                
                LOG_PTR("Health", result.data());
                return result;
            }

            ChCliEndurance GetEndurance() const { 
                LOG_MEMORY("ChCliCharacter", "GetEndurance", data(), Offsets::ChCliCharacter::ENDURANCE);
                
                ChCliEndurance result = ReadPointer<ChCliEndurance>(Offsets::ChCliCharacter::ENDURANCE);
                
                LOG_PTR("Endurance", result.data());
                return result;
            }

            ChCliEnergies GetEnergies() const { 
                LOG_MEMORY("ChCliCharacter", "GetEnergies", data(), Offsets::ChCliCharacter::ENERGIES);
                
                ChCliEnergies result = ReadPointer<ChCliEnergies>(Offsets::ChCliCharacter::ENERGIES);
                
                LOG_PTR("Energies", result.data());
                return result;
            }

            ChCliCoreStats GetCoreStats() const { 
                LOG_MEMORY("ChCliCharacter", "GetCoreStats", data(), Offsets::ChCliCharacter::CORE_STATS);
                
                ChCliCoreStats result = ReadPointer<ChCliCoreStats>(Offsets::ChCliCharacter::CORE_STATS);
                
                LOG_PTR("CoreStats", result.data());
                return result;
            }

            Game::Attitude GetAttitude() const {
                LOG_MEMORY("ChCliCharacter", "GetAttitude", data(), Offsets::ChCliCharacter::ATTITUDE);

                uint32_t attitudeValue = ReadMember<uint32_t>(Offsets::ChCliCharacter::ATTITUDE, 1);
                Game::Attitude attitude = static_cast<Game::Attitude>(attitudeValue);
                
                LOG_DEBUG("ChCliCharacter::GetAttitude - Attitude: %u", static_cast<uint32_t>(attitude));
                return attitude;
            }

            Game::CharacterRank GetRank() const {
                LOG_MEMORY("ChCliCharacter", "GetRank", data(), Offsets::ChCliCharacter::RANK_FLAGS);

                uint32_t flags = ReadMember<uint32_t>(Offsets::ChCliCharacter::RANK_FLAGS, 0);

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
                return ReadPointer<ChCliInventory>(Offsets::ChCliCharacter::INVENTORY);
            }

            ChCliSkillbar GetSkillbar() const {
                LOG_MEMORY("ChCliCharacter", "GetSkillbar", data(), Offsets::ChCliCharacter::SKILLBAR);
                ChCliSkillbar result = ReadPointer<ChCliSkillbar>(Offsets::ChCliCharacter::SKILLBAR);
                LOG_PTR("Skillbar", result.data());
                return result;
            }
        };

        /**
         * @brief Player wrapper that contains character data and player name
         */
        class ChCliPlayer : public SafeForeignClass {
        public:
            ChCliPlayer(void* ptr) : SafeForeignClass(ptr) {}
            
            ChCliCharacter GetCharacter() const { 
                if (!data()) {
                    return ChCliCharacter(nullptr);
                }
                
                void* characterPtr = nullptr;
                if (!kx::Debug::SafeRead<void*>(data(), Offsets::ChCliPlayer::CHARACTER_PTR, characterPtr)) {
                    return ChCliCharacter(nullptr);
                }
                
                return ChCliCharacter(characterPtr);
            }
            
            const wchar_t* GetName() const { 
                if (!data()) {
                    return nullptr;
                }
                
                wchar_t* namePtr = nullptr;
                if (!kx::Debug::SafeRead<wchar_t*>(data(), Offsets::ChCliPlayer::NAME_PTR, namePtr)) {
                    return nullptr;
                }
                
                return namePtr;
            }
        };

    } // namespace ReClass
} // namespace kx
