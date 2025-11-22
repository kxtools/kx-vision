#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Memory/SafeForeignClass.h"
#include "../GameEnums.h"
#include "HavokStructs.h"
#include <glm.hpp>

namespace kx {
    namespace ReClass {

        // Forward declarations
        class AgChar;
        class CoChar;
        class CoCharSimpleCliWrapper;
        class HkpSimpleShapePhantom;
        class HkpBoxShape;

        /**
         * @brief CoCharSimpleCliWrapper intermediate object accessed via CoChar->0x88 containing alternative positions
         * 
         * Note: PHYSICS_PHANTOM_PLAYER and BOX_SHAPE_NPC are entity-type specific.
         * Havok physics offsets are in HavokOffsets.h
         * 
         * TEST RESULTS:
         * - GetPositionAlt1(): Updates similarly to Primary - smooth and accurate
         * - GetPositionAlt2(): LAGS BEHIND visual position - not recommended for real-time rendering
         * - GetPhysicsPhantom()->GetPhysicsPosition(): Updates similarly to Primary - smooth and accurate
         */
        class CoCharSimpleCliWrapper : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t POSITION_ALT1 = 0xB8;    // glm::vec3 alternative position 1
                static constexpr uintptr_t POSITION_ALT2 = 0x118;   // glm::vec3 alternative position 2 (may lag)
                static constexpr uintptr_t PHYSICS_PHANTOM_PLAYER = 0x78;  // hkpSimpleShapePhantom* physics object (PLAYER ONLY) - see HavokOffsets.h
                static constexpr uintptr_t BOX_SHAPE_NPC = 0xE8;        // hkpBoxShape* physics box shape (NPC ONLY - Players are nullptr) - see HavokOffsets.h
            };

        public:
            CoCharSimpleCliWrapper(void* ptr) : SafeForeignClass(ptr) {}

            glm::vec3 GetPositionAlt1() const {
                // TESTED: Updates similarly to Primary position - smooth and accurate
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                return ReadMemberFast<glm::vec3>(Offsets::POSITION_ALT1, { 0.0f, 0.0f, 0.0f });
            }

            glm::vec3 GetPositionAlt2() const {
                // WARNING: TESTED - This position LAGS BEHIND the visual position
                // Not recommended for real-time rendering - causes visual delay
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                return ReadMemberFast<glm::vec3>(Offsets::POSITION_ALT2, { 0.0f, 0.0f, 0.0f });
            }

            HkpSimpleShapePhantom GetPhysicsPhantom() const {
                return ReadPointerFast<HkpSimpleShapePhantom>(Offsets::PHYSICS_PHANTOM_PLAYER);
            }

            HkpBoxShape GetBoxShapeNpc() const {
                return ReadPointerFast<HkpBoxShape>(Offsets::BOX_SHAPE_NPC);
            }
        };

        /**
         * @brief CoChar - Character coordinate system for visual positioning
         * VISUAL_POSITION is the primary position source for real-time rendering.
         */
        class CoChar : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t VISUAL_POSITION = 0x30;  // glm::vec3 position (primary)
                static constexpr uintptr_t RIGID_BODY_PLAYER = 0x60;       // hkpRigidBody* physics rigid body (PLAYER ONLY - NPCs are nullptr) - see HavokOffsets.h
                static constexpr uintptr_t SIMPLE_CLI_WRAPPER = 0x88;   // CoCharSimpleCliWrapper* - contains additional position data and physics info
            };

        public:
            CoChar(void* ptr) : SafeForeignClass(ptr) {}

            glm::vec3 GetVisualPosition() const {
                // TESTED: Primary position source - smooth and accurate for real-time rendering
                // Only log memory access for successful reads to reduce spam
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                
                glm::vec3 result = ReadMemberFast<glm::vec3>(Offsets::VISUAL_POSITION, { 0.0f, 0.0f, 0.0f });
                
                return result;
            }

            HkpRigidBody GetRigidBodyPlayer() const {
                return ReadPointerFast<HkpRigidBody>(Offsets::RIGID_BODY_PLAYER);
            }

            CoCharSimpleCliWrapper GetSimpleCliWrapper() const {
                return ReadPointerFast<CoCharSimpleCliWrapper>(Offsets::SIMPLE_CLI_WRAPPER);
            }
        };

        /**
         * @brief AgChar - Agent wrapper for characters
         */
        class AgChar : public SafeForeignClass {
        private:
            struct Offsets {
                static constexpr uintptr_t CO_CHAR = 0x50;  // CoChar* coordinate system
                static constexpr uintptr_t TYPE = 0x08;     // int32_t agent type identifier
                static constexpr uintptr_t ID = 0x0C;       // int32_t agent ID
                static constexpr uintptr_t GROUNDED_POSITION32 = 0x120;  // glm::vec3 last grounded/navmesh position (scaled by 32)
            };

        public:
            AgChar(void* ptr) : SafeForeignClass(ptr) {}

            CoChar GetCoChar() const {
                LOG_MEMORY("AgChar", "GetCoChar", data(), Offsets::CO_CHAR);
                
                CoChar result = ReadPointerFast<CoChar>(Offsets::CO_CHAR);
                
                LOG_PTR("CoChar", result.data());
                return result;
            }

            Game::AgentType GetType() const {
                LOG_MEMORY("AgChar", "GetType", data(), Offsets::TYPE);
                
                uint32_t type = ReadMemberFast<uint32_t>(Offsets::TYPE, 0);
                
                LOG_DEBUG("AgChar::GetType - Type: %u", type);
                return static_cast<Game::AgentType>(type);
            }

            int32_t GetId() const {
                LOG_MEMORY("AgChar", "GetId", data(), Offsets::ID);
                
                int32_t id = ReadMemberFast<int32_t>(Offsets::ID, 0);
                
                LOG_DEBUG("AgChar::GetId - ID: %d", id);
                return id;
            }

            /**
             * @brief Get last grounded/navmesh position (32-bit scaled coordinates)
             * @return Last position where entity was on ground/navmesh
             * @note Raw values are scaled by 32 (x/32, y/32, z/-32)
             * @note This position only updates when entity is grounded - does NOT update during jumps/falls
             * @note Useful for navmesh validation but NOT for real-time position tracking
             */
            glm::vec3 GetGroundedPosition32() const {
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                
                // Read the 32-bit scaled grounded position (last known ground/navmesh contact)
                glm::vec3 rawPos = ReadMemberFast<glm::vec3>(Offsets::GROUNDED_POSITION32, { 0.0f, 0.0f, 0.0f });
                
                // Convert from scaled coordinates to world coordinates
                // x and y are divided by 32, z is divided by -32 (inverted)
                return glm::vec3(
                    rawPos.x / 32.0f,
                    rawPos.y / 32.0f,
                    rawPos.z / -32.0f
                );
            }
        };

    } // namespace ReClass
} // namespace kx