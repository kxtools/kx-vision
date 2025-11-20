#pragma once

#include "../../Utils/SafeForeignClass.h"
#include "../../Utils/DebugLogger.h"
#include "../offsets.h"
#include <vector>

namespace kx {
    namespace ReClass {

        /**
         * @brief Wrapper for Skill Definitions (Static data)
         */
        class SkillDef : public SafeForeignClass {
        public:
            SkillDef(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetId() const {
                LOG_MEMORY("SkillDef", "GetId", data(), Offsets::SkillDef::ID);
                return ReadMember<uint32_t>(Offsets::SkillDef::ID, 0);
            }
        };

        /**
         * @brief Wrapper for a CharSkill node (Dynamic data)
         */
        class CharSkill : public SafeForeignClass {
        public:
            CharSkill(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetRechargeTimeMs() const {
                LOG_MEMORY("CharSkill", "GetRechargeTimeMs", data(), Offsets::CharSkill::RECHARGE_TIME_MS);
                return ReadMember<uint32_t>(Offsets::CharSkill::RECHARGE_TIME_MS, 0);
            }

            SkillDef GetSkillDef() const {
                LOG_MEMORY("CharSkill", "GetSkillDef", data(), Offsets::CharSkill::SKILL_DEF);
                return ReadPointer<SkillDef>(Offsets::CharSkill::SKILL_DEF);
            }
        };

        /**
         * @brief Info structure to return cooldown data easily
         */
        struct CooldownInfo {
            uint32_t skillId;
            float remainingSeconds;
            float maxCooldownSeconds; 
        };

        /**
         * @brief Wrapper for the internal mechanics of the skillbar
         */
        class CharSkillbar : public SafeForeignClass {
        public:
            CharSkillbar(void* ptr) : SafeForeignClass(ptr) {}

            uint32_t GetLastCastTime() const {
                LOG_MEMORY("CharSkillbar", "GetLastCastTime", data(), Offsets::CharSkillbar::LAST_CAST_TIME);
                return ReadMember<uint32_t>(Offsets::CharSkillbar::LAST_CAST_TIME, 0);
            }

            float GetRechargeRateScale() const {
                LOG_MEMORY("CharSkillbar", "GetRechargeRateScale", data(), Offsets::CharSkillbar::RECHARGE_RATE_SCALE);
                return ReadMember<float>(Offsets::CharSkillbar::RECHARGE_RATE_SCALE, 1.0f);
            }

            /**
             * @brief Iterates the m_rechargeList to find active cooldowns
             * @param currentWorldTime The 'worldTime' from AgApi->AgWorld
             */
            std::vector<CooldownInfo> GetActiveCooldowns(uint32_t currentWorldTime) const {
                std::vector<CooldownInfo> cooldowns;
                if (!data()) return cooldowns;

                uint32_t lastCast = GetLastCastTime();
                float scale = GetRechargeRateScale();
                
                float timeSinceCastScaled = (float)(currentWorldTime - lastCast) * scale;

                uintptr_t listBase = reinterpret_cast<uintptr_t>(data()) + Offsets::CharSkillbar::RECHARGE_LIST;
                uintptr_t terminatorAddr = listBase + 0x8;
                
                void* currentNodePtr = nullptr;
                kx::Debug::SafeRead<void*>(data(), Offsets::CharSkillbar::RECHARGE_LIST + 0x10, currentNodePtr);

                int safetyLimit = 50; 
                while (currentNodePtr != nullptr && 
                       reinterpret_cast<uintptr_t>(currentNodePtr) != terminatorAddr && 
                       safetyLimit-- > 0) 
                {
                    CharSkill skill(currentNodePtr);
                    
                    uint32_t rechargeTimeMs = skill.GetRechargeTimeMs();
                    float remainingMs = (float)rechargeTimeMs - timeSinceCastScaled;

                    if (remainingMs > 0.0f) {
                        CooldownInfo info;
                        SkillDef skillDef = skill.GetSkillDef();
                        info.skillId = skillDef.GetId();
                        info.remainingSeconds = remainingMs / 1000.0f;
                        info.maxCooldownSeconds = (float)rechargeTimeMs / 1000.0f;
                        
                        if (info.skillId != 0) {
                            cooldowns.push_back(info);
                        }
                    }

                    void* nextNode = nullptr;
                    if (!kx::Debug::SafeRead<void*>(currentNodePtr, 0x8, nextNode)) {
                        break;
                    }
                    currentNodePtr = nextNode;
                }

                return cooldowns;
            }
        };

    } // namespace ReClass
} // namespace kx

