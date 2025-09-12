#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Core/AppState.h"
#include "../../Utils/SafeForeignClass.h"
#include "../Coordinates.h"
#include "../offsets.h"

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

            Coordinates3D GetVisualPosition() {
                // Only log memory access for successful reads to reduce spam
                if (!data()) {
                    return { 0, 0, 0 };
                }
                
                Coordinates3D result = ReadMember<Coordinates3D>(Offsets::CO_CHAR_VISUAL_POSITION, { 0, 0, 0 });
                
                return result;
            }
        };

        /**
         * @brief Agent wrapper for character entities
         */
        class AgChar : public kx::SafeForeignClass {
        public:
            AgChar(void* ptr) : kx::SafeForeignClass(ptr) {}

            CoChar GetCoChar() {
                LOG_MEMORY("AgChar", "GetCoChar", data(), Offsets::AG_CHAR_CO_CHAR);
                
                CoChar result = ReadPointer<CoChar>(Offsets::AG_CHAR_CO_CHAR);
                
                LOG_PTR("CoChar", result.data());
                return result;
            }

            uint32_t GetType() {
                LOG_MEMORY("AgChar", "GetType", data(), Offsets::AG_CHAR_TYPE);
                
                uint32_t type = ReadMember<uint32_t>(Offsets::AG_CHAR_TYPE, 0);
                
                LOG_DEBUG("AgChar::GetType - Type: %u", type);
                return type;
            }
        };

    } // namespace ReClass
} // namespace kx