#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Utils/SafeForeignClass.h"
#include "../offsets.h"
#include <glm.hpp>

namespace kx {
    namespace ReClass {

        // Forward declarations
        class AgChar;
        class CoChar;
        class CoCharUnknown;
        class HkpSimpleShapePhantom;

        /**
         * @brief Havok physics phantom object - contains physics-simulated position
         * TESTED: Physics position updates similarly to Primary - smooth and accurate
         */
        class HkpSimpleShapePhantom : public SafeForeignClass {
        public:
            HkpSimpleShapePhantom(void* ptr) : SafeForeignClass(ptr) {}

            glm::vec3 GetPhysicsPosition() const {
                // TESTED: Updates similarly to Primary position - smooth and accurate
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                return ReadMember<glm::vec3>(Offsets::HkpSimpleShapePhantom::PHYSICS_POSITION, { 0.0f, 0.0f, 0.0f });
            }
        };

        /**
         * @brief Unknown object accessed via CoChar->0x88 containing alternative positions
         * 
         * TEST RESULTS:
         * - GetPositionAlt1(): Updates similarly to Primary - smooth and accurate
         * - GetPositionAlt2(): LAGS BEHIND visual position - not recommended for real-time rendering
         * - GetPhysicsPhantom()->GetPhysicsPosition(): Updates similarly to Primary - smooth and accurate
         */
        class CoCharUnknown : public SafeForeignClass {
        public:
            CoCharUnknown(void* ptr) : SafeForeignClass(ptr) {}

            glm::vec3 GetPositionAlt1() const {
                // TESTED: Updates similarly to Primary position - smooth and accurate
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                return ReadMember<glm::vec3>(Offsets::CoCharUnknown::POSITION_ALT1, { 0.0f, 0.0f, 0.0f });
            }

            glm::vec3 GetPositionAlt2() const {
                // WARNING: TESTED - This position LAGS BEHIND the visual position
                // Not recommended for real-time rendering - causes visual delay
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                return ReadMember<glm::vec3>(Offsets::CoCharUnknown::POSITION_ALT2, { 0.0f, 0.0f, 0.0f });
            }

            HkpSimpleShapePhantom GetPhysicsPhantom() const {
                return ReadPointer<HkpSimpleShapePhantom>(Offsets::CoCharUnknown::PHYSICS_PHANTOM);
            }
        };

        /**
         * @brief Coordinate/Object wrapper for character positioning
         */
        class CoChar : public SafeForeignClass {
        public:
            CoChar(void* ptr) : SafeForeignClass(ptr) {}

            glm::vec3 GetVisualPosition() const {
                // TESTED: Primary position source - smooth and accurate for real-time rendering
                // Only log memory access for successful reads to reduce spam
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                
                glm::vec3 result = ReadMember<glm::vec3>(Offsets::CoChar::VISUAL_POSITION, { 0.0f, 0.0f, 0.0f });
                
                return result;
            }

            CoCharUnknown GetUnknownObject() const {
                return ReadPointer<CoCharUnknown>(Offsets::CoChar::UNKNOWN_OBJECT);
            }
        };

        /**
         * @brief Agent wrapper for character entities
         */
        class AgChar : public SafeForeignClass {
        public:
            AgChar(void* ptr) : SafeForeignClass(ptr) {}

            CoChar GetCoChar() const {
                LOG_MEMORY("AgChar", "GetCoChar", data(), Offsets::AgChar::CO_CHAR);
                
                CoChar result = ReadPointer<CoChar>(Offsets::AgChar::CO_CHAR);
                
                LOG_PTR("CoChar", result.data());
                return result;
            }

            uint32_t GetType() const {
                LOG_MEMORY("AgChar", "GetType", data(), Offsets::AgChar::TYPE);
                
                uint32_t type = ReadMember<uint32_t>(Offsets::AgChar::TYPE, 0);
                
                LOG_DEBUG("AgChar::GetType - Type: %u", type);
                return type;
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
                glm::vec3 rawPos = ReadMember<glm::vec3>(Offsets::AgChar::GROUNDED_POSITION32, { 0.0f, 0.0f, 0.0f });
                
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