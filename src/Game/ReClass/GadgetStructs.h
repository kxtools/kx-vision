#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Utils/SafeForeignClass.h"
#include "../GameEnums.h"
#include "../offsets.h"
#include "CharacterStructs.h"
#include "AgentStructs.h"
#include <glm.hpp>

namespace kx {
    namespace ReClass {

        // Forward declarations
        class CoKeyFramed;
        class AgKeyFramed;
        class GdCliGadget;
        class HkpRigidBody;
        class HkpCylinderShape;
        class HkpBoxShape;
        class AgentInl;

        /**
         * @brief Havok physics cylinder collision shape - contains gadget dimensions
         */
        class HkpCylinderShape : public SafeForeignClass {
        public:
            HkpCylinderShape(void* ptr) : SafeForeignClass(ptr) {}

            int32_t GetHeightCentimeters() const {
                if (!data()) {
                    return 0;
                }
                return ReadMember<int32_t>(Offsets::HkpCylinderShape::HEIGHT_CM, 0);
            }
        };

        /**
         * @brief Havok physics rigid body - contains physics shape reference
         */
        class HkpRigidBody : public SafeForeignClass {
        public:
            HkpRigidBody(void* ptr) : SafeForeignClass(ptr) {}

            HkpCylinderShape GetCylinderShape() const {
                return ReadPointer<HkpCylinderShape>(Offsets::HkpRigidBody::SHAPE);
            }

            HkpBoxShape GetBoxShape() const {
                return ReadPointer<HkpBoxShape>(Offsets::HkpRigidBody::SHAPE);
            }
        };

        /**
         * @brief Coordinate/Object wrapper for keyframed entities (gadgets)
         */
        class CoKeyFramed : public SafeForeignClass {
        public:
            CoKeyFramed(void* ptr) : SafeForeignClass(ptr) {}

            glm::vec3 GetPosition() const {
                LOG_MEMORY("CoKeyFramed", "GetPosition", data(), Offsets::CoKeyframed::POSITION);
                
                glm::vec3 position = ReadMember<glm::vec3>(Offsets::CoKeyframed::POSITION, glm::vec3{0.0f, 0.0f, 0.0f});
                
                LOG_DEBUG("CoKeyFramed::GetPosition - Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
                return position;
            }

            HkpRigidBody GetRigidBody() const {
                return ReadPointer<HkpRigidBody>(Offsets::CoKeyframed::RIGID_BODY);
            }
        };

        /**
         * @brief Agent wrapper for keyframed entities
         */
        class AgKeyFramed : public SafeForeignClass {
        public:
            AgKeyFramed(void* ptr) : SafeForeignClass(ptr) {}

            CoKeyFramed GetCoKeyFramed() const {
                LOG_MEMORY("AgKeyFramed", "GetCoKeyFramed", data(), Offsets::AgKeyframed::CO_KEYFRAMED);
                
                CoKeyFramed result = ReadPointer<CoKeyFramed>(Offsets::AgKeyframed::CO_KEYFRAMED);
                
                LOG_PTR("CoKeyFramed", result.data());
                return result;
            }

            Game::AgentType GetType() const {
                LOG_MEMORY("AgKeyFramed", "GetType", data(), Offsets::AgKeyframed::TYPE);
                
                uint32_t type = ReadMember<uint32_t>(Offsets::AgKeyframed::TYPE, 0);
                
                LOG_DEBUG("AgKeyFramed::GetType - Type: %u", type);
                return static_cast<Game::AgentType>(type);
            }

            int32_t GetId() const {
                LOG_MEMORY("AgKeyFramed", "GetId", data(), Offsets::AgKeyframed::ID);
                
                int32_t id = ReadMember<int32_t>(Offsets::AgKeyframed::ID, 0);
                
                LOG_DEBUG("AgKeyFramed::GetId - ID: %d", id);
                return id;
            }
        };

        /**
         * @brief Client gadget wrapper
         */
        class GdCliGadget : public SafeForeignClass {
        public:
            GdCliGadget(void* ptr) : SafeForeignClass(ptr) {}

            Game::GadgetType GetGadgetType() const {
                LOG_MEMORY("GdCliGadget", "GetGadgetType", data(), Offsets::GdCliGadget::TYPE);
                
                uint32_t typeValue = ReadMember<uint32_t>(Offsets::GdCliGadget::TYPE, 0);
                Game::GadgetType gadgetType = static_cast<Game::GadgetType>(typeValue);
                
                LOG_DEBUG("GdCliGadget::GetGadgetType - Type: %u", static_cast<uint32_t>(gadgetType));
                return gadgetType;
            }

            ChCliHealth GetHealth() const {
                LOG_MEMORY("GdCliGadget", "GetHealth", data(), Offsets::GdCliGadget::HEALTH);

                ChCliHealth result = ReadPointer<ChCliHealth>(Offsets::GdCliGadget::HEALTH);

                LOG_PTR("Health", result.data());
                return result;
            }

            Game::ResourceNodeType GetResourceNodeType() const {
                LOG_MEMORY("GdCliGadget", "GetResourceNodeType", data(), Offsets::GdCliGadget::RESOURCE_NODE_TYPE);

                return ReadMember<Game::ResourceNodeType>(Offsets::GdCliGadget::RESOURCE_NODE_TYPE, Game::ResourceNodeType::None);
            }

            bool IsGatherable() const {
                LOG_MEMORY("GdCliGadget", "IsGatherable", data(), Offsets::GdCliGadget::FLAGS);
                
                uint32_t flags = ReadMember<uint32_t>(Offsets::GdCliGadget::FLAGS, 0);
                bool gatherable = (flags & Offsets::GdCliGadget::FLAG_GATHERABLE) != 0;
                
                LOG_DEBUG("GdCliGadget::IsGatherable - Flags: 0x%X, Gatherable: %s", flags, gatherable ? "true" : "false");
                return gatherable;
            }

            AgKeyFramed GetAgKeyFramed() const {
                LOG_MEMORY("GdCliGadget", "GetAgKeyFramed", data(), Offsets::GdCliGadget::AG_KEYFRAMED);
                
                AgKeyFramed result = ReadPointer<AgKeyFramed>(Offsets::GdCliGadget::AG_KEYFRAMED);
                
                LOG_PTR("AgKeyFramed", result.data());
                return result;
            }
        };

        /**
         * @brief AgentInl - Internal agent structure wrapper for attack targets
         * 
         * Internal class: Gw2::Engine::Agent::AgentInl
         * Used in the attack target list (walls, destructible objects, etc.)
         * Contains position, health, combat state, and defeat status information.
         */
        class AgentInl : public SafeForeignClass {
        public:
            AgentInl(void* ptr) : SafeForeignClass(ptr) {}

            AgKeyFramed GetAgKeyFramed() const {
                LOG_MEMORY("AgentInl", "GetAgKeyFramed", data(), Offsets::AgentInl::AG_KEYFRAMED);
                
                AgKeyFramed result = ReadPointer<AgKeyFramed>(Offsets::AgentInl::AG_KEYFRAMED);
                
                LOG_PTR("AgKeyFramed", result.data());
                return result;
            }

            Game::AttackTargetCombatState GetCombatState() const {
                LOG_MEMORY("AgentInl", "GetCombatState", data(), Offsets::AgentInl::COMBAT_STATE);
                
                int32_t state = ReadMember<int32_t>(Offsets::AgentInl::COMBAT_STATE, 0);
                
                LOG_DEBUG("AgentInl::GetCombatState - State: %d", state);
                return static_cast<Game::AttackTargetCombatState>(state);
            }
        };

    } // namespace ReClass
} // namespace kx