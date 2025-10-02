#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Utils/SafeForeignClass.h"
#include "../offsets.h"
#include <glm.hpp>

namespace kx {
    namespace ReClass {

        // Forward declarations
        class AgChar;

        /**
         * @brief Coordinate/Object wrapper for character positioning
         */
        class CoChar : public kx::SafeForeignClass {
        public:
            CoChar(void* ptr) : kx::SafeForeignClass(ptr) {}

            glm::vec3 GetVisualPosition() const {
                // Only log memory access for successful reads to reduce spam
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                
                glm::vec3 result = ReadMember<glm::vec3>(Offsets::CoChar::VISUAL_POSITION, { 0.0f, 0.0f, 0.0f });
                
                return result;
            }
        };

        /**
         * @brief Agent wrapper for character entities
         */
        class AgChar : public kx::SafeForeignClass {
        public:
            AgChar(void* ptr) : kx::SafeForeignClass(ptr) {}

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
        };

    } // namespace ReClass
} // namespace kx