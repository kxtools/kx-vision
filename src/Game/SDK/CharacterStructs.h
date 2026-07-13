#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Memory/ForeignClass.h"
#include "../GameEnums.h"
#include "AgentStructs.h"
#include "EquipmentStructs.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief ChCliHealth - Character health management
         */
        class ChCliHealth : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CURRENT = 0x0C;  // float current health
                static constexpr uintptr_t MAX = 0x10;      // float maximum health
                static constexpr uintptr_t HEALTH_REGEN_RATE = 0x14; // float health regeneration rate (0 in combat, often 10% of max HP otherwise)
                static constexpr uintptr_t BARRIER = 0x28;  // float current barrier
            };

        public:
            ChCliHealth(void* ptr) : ForeignClass(ptr) {}
            
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
        class ChCliEnergies : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CURRENT = 0x0C;  // float current energy
                static constexpr uintptr_t MAX = 0x10;      // float maximum energy
            };

        public:
            ChCliEnergies(void* ptr) : ForeignClass(ptr) {}
            
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
        class ChCliEndurance : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CURRENT = 0x10;  // float current endurance
                static constexpr uintptr_t MAX = 0x14;      // float maximum endurance
            };

        public:
            ChCliEndurance(void* ptr) : ForeignClass(ptr) {}
            
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
        class ChCliCoreStats : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t RACE = 0x33;          // uint8_t race ID
                static constexpr uintptr_t LEVEL = 0xAC;         // uint32_t actual level
                static constexpr uintptr_t PROFESSION = 0x12C;   // uint32_t profession ID
                static constexpr uintptr_t SCALED_LEVEL = 0x234; // uint32_t scaled/effective level
            };

        public:
            ChCliCoreStats(void* ptr) : ForeignClass(ptr) {}
            
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

        class ChCliSkillbar;

        /**
         * @brief ChCliCharacter - Main character structure containing all subsystems
         */
        class ChCliCharacter : public ForeignClass {
        private:
            struct Offsets {
                // --- States ---
                static constexpr uintptr_t WATER_STATE = 0x0D8;    // uint32_t: 0=Dry, 1=Diving, 2=Surface
                static constexpr uintptr_t LIFE_STATE = 0x0188;     // uint32_t: 0=Alive, 1=Dead, 2=Downed

                // --- Identification ---
                static constexpr uintptr_t AGENT = 0x98;            // AgChar* character's agent
                static constexpr uintptr_t ATTITUDE = 0x00C0;       // uint32_t attitude flags

                // --- Subsystems ---
                static constexpr uintptr_t BREAKBAR = 0x00C8;       // CmbtCliBreakBar* breakbar subsystem
                static constexpr uintptr_t RANK_FLAGS = 0x0264;     // uint32_t rank flags (veteran, elite, etc.)
                static constexpr uintptr_t CORE_STATS = 0x0388;     // ChCliCoreStats* stats subsystem
                static constexpr uintptr_t ENDURANCE = 0x03D0;      // ChCliEndurance* dodge/endurance subsystem
                static constexpr uintptr_t ENERGIES = 0x03D8;       // ChCliEnergies* mount/special energy subsystem
                static constexpr uintptr_t FORCE = 0x03E0;          // ChCliForce* force subsystem
                static constexpr uintptr_t HEALTH = 0x03E8;         // ChCliHealth* health subsystem
                static constexpr uintptr_t INVENTORY = 0x3F0;       // ChCliInventory* inventory subsystem
                static constexpr uintptr_t SKILLBAR = 0x0520;       // ChCliSkillbar* skillbar subsystem
				static constexpr uintptr_t WEAPON_DRAWN = 0x0178;   // uint8_t weapon drawn (24=sheathed, 88=drawn)
            };

        public:
            ChCliCharacter(void* ptr) : ForeignClass(ptr) {}

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

            bool IsWeaponDrawn() const {
                LOG_MEMORY("ChCliCharacter", "IsWeaponDrawn", data(), Offsets::WEAPON_DRAWN);
                uint8_t drawnState = ReadMemberFast<uint8_t>(Offsets::WEAPON_DRAWN, 0);
                
                LOG_DEBUG("ChCliCharacter::IsWeaponDrawn - Drawn State: %u", drawnState);
                return drawnState == 88; // 88 = drawn, 24 = sheathed
			}

            uint32_t GetWaterState() const {
                LOG_MEMORY("ChCliCharacter", "GetWaterState", data(), Offsets::WATER_STATE);
                
                uint32_t waterState = ReadMemberFast<uint32_t>(Offsets::WATER_STATE, 0);
                
                LOG_DEBUG("ChCliCharacter::GetWaterState - Water State: %u", waterState);
                return waterState;
			}

            uint32_t GetLifeState() const {
                LOG_MEMORY("ChCliCharacter", "GetLifeState", data(), Offsets::LIFE_STATE);
                
                uint32_t lifeState = ReadMemberFast<uint32_t>(Offsets::LIFE_STATE, 0);
                
                LOG_DEBUG("ChCliCharacter::GetLifeState - Life State: %u", lifeState);
                return lifeState;
            }

            ChCliSkillbar GetSkillbar() const;
        };

        // Forward declarations
        class AsContext;
        class CharSkillbar;

        /**
         * @brief ChCliSkill - Skill definition entry for a skillbar slot
         */
        class ChCliSkill : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t SKILL_ID = 0x0028;       // uint32_t skill id
                static constexpr uintptr_t FLAGS = 0x0038;          // uint32_t status/behavioral bitmask: disabled/silenced state (e.g. 0x1000), targeting requirements, and special range-check requirements (e.g. 0x1000000)
            };

        public:
            ChCliSkill(void* ptr) : ForeignClass(ptr) {}

            uint32_t GetSkillId() const {
                return ReadMemberFast<uint32_t>(Offsets::SKILL_ID, 0u);
            }

            uint32_t GetFlags() const {
                return ReadMemberFast<uint32_t>(Offsets::FLAGS, 0u);
            }
        };

        /**
         * @brief ChCliSkillbar - Character skillbar subsystem
         */
        class ChCliSkillbar : public ForeignClass {
        private:
            static constexpr uintptr_t SLOT_COUNT = 23;

            struct Offsets {
                static constexpr uintptr_t SLOT_VALID = 0x0074;                              // uint32_t active slot index; 23 (0x17) means "no skill active", 0..22 are valid slots
                static constexpr uintptr_t P_AS_CONTEXT = 0x0080;                             // AsContext* as context
                static constexpr uintptr_t USING_SKILL = 0x00B0;                                // uint32_t
                static constexpr uintptr_t P_CH_CLI_CHARACTER = 0x00F0;                      // ChCliCharacter* character
                static constexpr uintptr_t NOTIFIER = 0x0100;                                 // void* notifier (N00002074*)
                static constexpr uintptr_t P_CHAR_SKILLBAR = 0x0150;                          // void* char skillbar (CharSkillbar*)
                static constexpr uintptr_t REGISTERED_COMBATANT = 0x0158;                     // void* registered combatant (N00001FE8*)
                static constexpr uintptr_t REGISTERED_CORE_STATS = 0x0160;                    // ChCliCoreStats* registered core stats
                static constexpr uintptr_t REGISTERED_INVENTORY = 0x0168;                     // ChCliInventory* registered inventory
                static constexpr uintptr_t REGISTERED_TRANSFORMATION_MGR = 0x0178;           // void* registered transformation manager (N00002023*)
                static constexpr uintptr_t HELD_INSTANT_SKILL_SLOT = 0x0180;                  // uint32_t/hex slot indicator while the key is held down; reports 0..22 for supported instant/non-ground skills, otherwise inactive
                static constexpr uintptr_t SLOT_SKILL_DEFS = 0x01D0;                         // ChCliSkill*[23] slot skill definitions
            };

        public:
            ChCliSkillbar(void* ptr) : ForeignClass(ptr) {}

            void* GetAsContext() const {
                LOG_MEMORY("ChCliSkillbar", "GetAsContext", data(), Offsets::P_AS_CONTEXT);
                return ReadMemberFast<void*>(Offsets::P_AS_CONTEXT, nullptr);
            }

            uint32_t GetUsingSkill() const {
                return ReadMemberFast<uint32_t>(Offsets::USING_SKILL, 0u);
            }

            ChCliSkill GetSlotSkillDef(uint32_t slotIndex) const {
                if (slotIndex >= SLOT_COUNT || !data()) {
                    return ChCliSkill(nullptr);
                }

                return ReadPointerFast<ChCliSkill>(Offsets::SLOT_SKILL_DEFS + (slotIndex * sizeof(void*)));
            }
        };

        inline ChCliSkillbar ChCliCharacter::GetSkillbar() const {
            return ReadPointerFast<ChCliSkillbar>(Offsets::SKILLBAR);
        }

        /**
         * @brief ChCliPlayer - Player wrapper containing character and name
         */
        class ChCliPlayer : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CHARACTER_PTR = 0x18;  // ChCliCharacter* player's character
                static constexpr uintptr_t NAME_PTR = 0x68;       // wchar_t* player name string
            };

        public:
            ChCliPlayer(void* ptr) : ForeignClass(ptr) {}
            
            ChCliCharacter GetCharacter() const { 
                return ReadPointerFast<ChCliCharacter>(Offsets::CHARACTER_PTR);
            }
            
            const wchar_t* GetName() const { 
                return ReadMemberFast<const wchar_t*>(Offsets::NAME_PTR, nullptr);
            }
        };

    } // namespace ReClass
} // namespace kx
