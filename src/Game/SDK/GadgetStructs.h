#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Memory/ForeignClass.h"
#include "../GameEnums.h"
#include "HavokStructs.h"
#include <glm.hpp>

namespace kx {
    namespace ReClass {

        // Forward declarations
        class CoKeyFramed;
        class AgKeyFramed;
        class GdCliGadget;
        class AgentInl;

        /**
         * @brief CoKeyFramed - Coordinate system for keyframed objects (gadgets)
         */
        class CoKeyFramed : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t POSITION = 0x0030;  // glm::vec3 position
                static constexpr uintptr_t RIGID_BODY = 0x0060; // hkpRigidBody* physics rigid body (gadgets only) - see HavokOffsets.h
                static constexpr uintptr_t ROTATION = 0x00F8; // glm::vec2 rotation (gadget rotation)
            };

        public:
            CoKeyFramed(void* ptr) : ForeignClass(ptr) {}

            glm::vec3 GetPosition() const {
                LOG_MEMORY("CoKeyFramed", "GetPosition", data(), Offsets::POSITION);
                
                glm::vec3 position = ReadMemberFast<glm::vec3>(Offsets::POSITION, glm::vec3{0.0f, 0.0f, 0.0f});
                
                LOG_DEBUG("CoKeyFramed::GetPosition - Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
                return position;
            }

            HkpRigidBody GetRigidBody() const {
                return ReadPointerFast<HkpRigidBody>(Offsets::RIGID_BODY);
            }
        };

        /**
         * @brief AgKeyFramed - Agent wrapper for keyframed objects (gadgets)
         *
         * TYPE values:
         * - 10: Regular gadget (AgentType::Gadget)
         * - 11: Attack target (AgentType::GadgetAttackTarget) - walls, destructible objects
         */
        class AgKeyFramed : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t TYPE = 0x08;            // int32_t agent type identifier
                static constexpr uintptr_t ID = 0x0C;              // int32_t agent ID
                static constexpr uintptr_t GADGET_TYPE = 0x40;     // uint32_t gadget type
                static constexpr uintptr_t CO_KEYFRAMED = 0x0050;  // CoKeyframed* coordinate system
            };

        public:
            AgKeyFramed(void* ptr) : ForeignClass(ptr) {}

            CoKeyFramed GetCoKeyFramed() const {
                LOG_MEMORY("AgKeyFramed", "GetCoKeyFramed", data(), Offsets::CO_KEYFRAMED);
                
                CoKeyFramed result = ReadPointerFast<CoKeyFramed>(Offsets::CO_KEYFRAMED);
                
                LOG_PTR("CoKeyFramed", result.data());
                return result;
            }

            Game::AgentType GetType() const {
                LOG_MEMORY("AgKeyFramed", "GetType", data(), Offsets::TYPE);
                
                uint32_t type = ReadMemberFast<uint32_t>(Offsets::TYPE, 0);
                
                LOG_DEBUG("AgKeyFramed::GetType - Type: %u", type);
                return static_cast<Game::AgentType>(type);
            }

            int32_t GetId() const {
                LOG_MEMORY("AgKeyFramed", "GetId", data(), Offsets::ID);
                
                int32_t id = ReadMemberFast<int32_t>(Offsets::ID, 0);
                
                LOG_DEBUG("AgKeyFramed::GetId - ID: %d", id);
                return id;
            }
        };

        /**
         * @brief GdCliHealth - Gadget health management wrapper (current and max HP only)
         */
        class GdCliHealth : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CURRENT = 0x0C;  // float current health
                static constexpr uintptr_t MAX = 0x10;      // float maximum health
            };

        public:
            GdCliHealth(void* ptr) : ForeignClass(ptr) {}
            
            float GetCurrent() const { 
                LOG_MEMORY("GdCliHealth", "GetCurrent", data(), Offsets::CURRENT);
                
                float current = ReadMemberFast<float>(Offsets::CURRENT, 0.0f);
                
                LOG_DEBUG("GdCliHealth::GetCurrent - Current: %.2f", current);
                return current;
            }
            
            float GetMax() const { 
                LOG_MEMORY("GdCliHealth", "GetMax", data(), Offsets::MAX);
                
                float max = ReadMemberFast<float>(Offsets::MAX, 0.0f);
                
                LOG_DEBUG("GdCliHealth::GetMax - Max: %.2f", max);
                return max;
            }
        };

        /**
         * @brief GdCliGadget - Game gadget/object structure
         */
        class GdCliGadget : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t AG_KEYFRAMED = 0x0038;         // AgKeyframed* agent wrapper
                static constexpr uintptr_t TYPE = 0x0208;                 // uint32_t gadget type
                static constexpr uintptr_t HEALTH = 0x0220;               // GdCliHealth* health subsystem
                static constexpr uintptr_t RESOURCE_NODE_TYPE = 0x04EC;   // uint32_t resource node type
                static constexpr uintptr_t FLAGS = 0x04F0;                // uint32_t gadget flags
            };
            static constexpr uint32_t FlagGatherable = 0x2;  // Indicates gatherable resource

        public:
            GdCliGadget(void* ptr) : ForeignClass(ptr) {}

            Game::GadgetType GetGadgetType() const {
                LOG_MEMORY("GdCliGadget", "GetGadgetType", data(), Offsets::TYPE);
                
                uint32_t typeValue = ReadMemberFast<uint32_t>(Offsets::TYPE, 0);
                Game::GadgetType gadgetType = static_cast<Game::GadgetType>(typeValue);
                
                LOG_DEBUG("GdCliGadget::GetGadgetType - Type: %u", static_cast<uint32_t>(gadgetType));
                return gadgetType;
            }

            GdCliHealth GetHealth() const {
                LOG_MEMORY("GdCliGadget", "GetHealth", data(), Offsets::HEALTH);

                GdCliHealth result = ReadPointerFast<GdCliHealth>(Offsets::HEALTH);

                LOG_PTR("Health", result.data());
                return result;
            }

            Game::ResourceNodeType GetResourceNodeType() const {
                LOG_MEMORY("GdCliGadget", "GetResourceNodeType", data(), Offsets::RESOURCE_NODE_TYPE);

                return ReadMemberFast<Game::ResourceNodeType>(Offsets::RESOURCE_NODE_TYPE, Game::ResourceNodeType::None);
            }

            bool IsGatherable() const {
                LOG_MEMORY("GdCliGadget", "IsGatherable", data(), Offsets::FLAGS);
                
                uint32_t flags = ReadMemberFast<uint32_t>(Offsets::FLAGS, 0);
                bool gatherable = (flags & FlagGatherable) != 0;
                
                LOG_DEBUG("GdCliGadget::IsGatherable - Flags: 0x%X, Gatherable: %s", flags, gatherable ? "true" : "false");
                return gatherable;
            }

            AgKeyFramed GetAgKeyFramed() const {
                LOG_MEMORY("GdCliGadget", "GetAgKeyFramed", data(), Offsets::AG_KEYFRAMED);
                
                AgKeyFramed result = ReadPointerFast<AgKeyFramed>(Offsets::AG_KEYFRAMED);
                
                LOG_PTR("AgKeyFramed", result.data());
                return result;
            }
        };

        /**
         * @brief AgentInl - Internal agent structure wrapper for attack targets
         * 
         * Internal class: Gw2::Engine::Agent::AgentInl
         * Used in the attack target list (walls, destructible objects, etc.)
         * Entries point to AgKeyframed with TYPE=11 (GadgetAttackTarget)
         * Contains position, health, combat state, and defeat status information.
         */
        class AgentInl : public ForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t AG_KEYFRAMED = 0x18;    // AgKeyframed* agent wrapper
                static constexpr uintptr_t COMBAT_STATE = 0x0034;  // int32_t combat state flag (2=Idle, 3=In Combat) [CONFIRMED]
            };

        public:
            AgentInl(void* ptr) : ForeignClass(ptr) {}

            AgKeyFramed GetAgKeyFramed() const {
                LOG_MEMORY("AgentInl", "GetAgKeyFramed", data(), Offsets::AG_KEYFRAMED);
                
                AgKeyFramed result = ReadPointerFast<AgKeyFramed>(Offsets::AG_KEYFRAMED);
                
                LOG_PTR("AgKeyFramed", result.data());
                return result;
            }

            Game::AttackTargetCombatState GetCombatState() const {
                LOG_MEMORY("AgentInl", "GetCombatState", data(), Offsets::COMBAT_STATE);
                
                int32_t state = ReadMemberFast<int32_t>(Offsets::COMBAT_STATE, 0);
                
                LOG_DEBUG("AgentInl::GetCombatState - State: %d", state);
                return static_cast<Game::AttackTargetCombatState>(state);
            }
        };

    } // namespace ReClass
} // namespace kx