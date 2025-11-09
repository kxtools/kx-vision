#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Utils/SafeForeignClass.h"
#include "../GameEnums.h"
#include "../HavokEnums.h"
#include "../offsets.h"
#include "../HavokOffsets.h"
#include "CharacterStructs.h"
#include "AgentStructs.h"
#include <glm.hpp>
#include <cmath>

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

            // Deprecated: Use HkpRigidBody::TryGetHeightMeters() for type-safe dimension extraction
            [[deprecated("Use HkpRigidBody::TryGetHeightMeters() for type-safe dimension extraction")]]
            float GetHeightHalfMeters() const {
                if (!data()) {
                    return 0.0f;
                }
                return ReadMember<float>(HavokOffsets::HkpCylinderShape::HEIGHT_HALF_FLOAT, 0.0f);
            }
        };

        /**
         * @brief Havok physics rigid body - contains physics shape reference
         * 
         * Type-safe shape dimension extraction using primitive shape type byte at shape+0x10.
         * This prevents unsafe casts and reading incorrect fields from wrong shape types.
         */
        class HkpRigidBody : public SafeForeignClass {
        public:
            HkpRigidBody(void* ptr) : SafeForeignClass(ptr) {}

            /**
             * @brief Get the wrapper shape type from the rigid body (for future filtering/early-out)
             * @return Wrapper type enum value, or INVALID if read fails
             * @note This reads from HkpRigidBody + 0x4C and can be used for early filtering
             */
            Havok::HkcdShapeType GetShapeTypeWrapper() const {
                if (!data()) {
                    return Havok::HkcdShapeType::INVALID;
                }
                uint8_t typeValue = ReadMember<uint8_t>(HavokOffsets::HkpRigidBody::SHAPE_TYPE_WRAPPER, 0xFF);
                return static_cast<Havok::HkcdShapeType>(typeValue);
            }

            /**
             * @brief Get the primitive shape type identifier from the shape object
             * @return Primitive shape type enum value, or INVALID if read fails
             * @note This reads the single byte at shape + 0x10, which is the actual primitive type
             */
            Havok::HkcdShapeType GetShapeType() const {
                if (!data()) {
                    return Havok::HkcdShapeType::INVALID;
                }

                // Read shape pointer at +0x20
                void* shapePtr = nullptr;
                if (!kx::Debug::SafeRead<void*>(data(), HavokOffsets::HkpRigidBody::SHAPE, shapePtr)) {
                    return Havok::HkcdShapeType::INVALID;
                }

                // Validate shape pointer (null check only - SafeRead handles memory safety)
                if (!shapePtr) {
                    return Havok::HkcdShapeType::INVALID;
                }

                // Read primitive shape type from shape + 0x10 (single byte)
                uint8_t typeValue = 0xFF;
                if (!kx::Debug::SafeRead<uint8_t>(shapePtr, HavokOffsets::HkpShapeBase::SHAPE_TYPE_PRIMITIVE, typeValue)) {
                    return Havok::HkcdShapeType::INVALID;
                }

                return static_cast<Havok::HkcdShapeType>(typeValue);
            }

        private:
            // Helper: Read int32 height in centimeters, convert to meters
            float ReadInt32HeightCm(void* shapePtr, uintptr_t offset, int32_t minCm, int32_t maxCm) const {
                int32_t heightCm = 0;
                if (!kx::Debug::SafeRead<int32_t>(shapePtr, offset, heightCm)) {
                    return -1.0f;
                }
                
                if (heightCm < minCm || heightCm > maxCm) {
                    return -1.0f;
                }
                
                return heightCm / 100.0f;
            }

            // Helper: Read float half-extent in game coordinates, convert to full height in meters
            float ReadFloatHeightHalfExtent(void* shapePtr, uintptr_t offset) const {
                float heightHalf = 0.0f;
                if (!kx::Debug::SafeRead<float>(shapePtr, offset, heightHalf)) {
                    return -1.0f;
                }
                
                if (!std::isfinite(heightHalf) || heightHalf <= 0.0f || heightHalf > 10000.0f) {
                    return -1.0f;
                }
                
                float fullHeightMeters = (heightHalf * 2.0f) / 1.23f;
                
                if (fullHeightMeters < 0.1f || fullHeightMeters > 100.0f) {
                    return -1.0f;
                }
                
                return fullHeightMeters;
            }

            // Helper: Read float half-height in meters, convert to full height (no coordinate conversion)
            float ReadFloatHeightHalfMeters(void* shapePtr, uintptr_t offset) const {
                float heightHalf = 0.0f;
                if (!kx::Debug::SafeRead<float>(shapePtr, offset, heightHalf)) {
                    return -1.0f;
                }
                
                if (!std::isfinite(heightHalf) || heightHalf <= 0.0f || heightHalf > 100.0f) {
                    return -1.0f;
                }
                
                float fullHeightMeters = heightHalf * 2.0f;
                
                if (fullHeightMeters < 0.1f || fullHeightMeters > 200.0f) {
                    return -1.0f;
                }
                
                return fullHeightMeters;
            }

            // Helper: Read int32 height directly (no conversion)
            float ReadInt32HeightDirect(void* shapePtr, uintptr_t offset, int32_t min, int32_t max) const {
                int32_t height = 0;
                if (!kx::Debug::SafeRead<int32_t>(shapePtr, offset, height)) {
                    return -1.0f;
                }
                
                if (height < min || height > max) {
                    return -1.0f;
                }
                
                return static_cast<float>(height);
            }

        public:
            /**
             * @brief Type-safe height extraction from rigid body shape
             * @return Height in meters, or -1.0f if shape type is unsupported or invalid
             * 
             * Supports CYLINDER, BOX, and CAPSULE shapes.
             * All other shape types return -1.0f to indicate unsupported.
             */
            float TryGetHeightMeters() const {
                if (!data()) {
                    return -1.0f;
                }

                // Read shape pointer at +0x20
                void* shapePtr = nullptr;
                if (!kx::Debug::SafeRead<void*>(data(), HavokOffsets::HkpRigidBody::SHAPE, shapePtr)) {
                    return -1.0f;
                }

                // Validate shape pointer (null check only - SafeRead handles memory safety)
                if (!shapePtr) {
                    return -1.0f;
                }

                // Read primitive shape type from shape + 0x10 (single byte)
                Havok::HkcdShapeType shapeType = GetShapeType();
                if (shapeType == Havok::HkcdShapeType::INVALID) {
                    return -1.0f;
                }

                // Switch on primitive shape type and extract height from appropriate field
                switch (shapeType) {
                    case Havok::HkcdShapeType::CYLINDER:
                        return ReadFloatHeightHalfMeters(shapePtr, HavokOffsets::HkpCylinderShape::HEIGHT_HALF_FLOAT);
                    
                    case Havok::HkcdShapeType::BOX:
                        return ReadFloatHeightHalfExtent(shapePtr, HavokOffsets::HkpBoxShape::HEIGHT_HALF);
                    
                    case Havok::HkcdShapeType::CAPSULE:
                        return ReadInt32HeightDirect(shapePtr, HavokOffsets::HkpCapsuleShape::HEIGHT, 1, 100);
                    
                    default:
                        return -1.0f;
                }
            }

            // Deprecated: Unsafe shape casting methods - use TryGetHeightMeters() instead
            // These methods assume the shape is the expected type without verification
            [[deprecated("Use TryGetHeightMeters() for type-safe dimension extraction")]]
            HkpCylinderShape GetCylinderShape() const {
                return ReadPointer<HkpCylinderShape>(HavokOffsets::HkpRigidBody::SHAPE);
            }

            [[deprecated("Use TryGetHeightMeters() for type-safe dimension extraction")]]
            HkpBoxShape GetBoxShape() const {
                return ReadPointer<HkpBoxShape>(HavokOffsets::HkpRigidBody::SHAPE);
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