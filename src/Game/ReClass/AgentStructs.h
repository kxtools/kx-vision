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
        class CoChar : public SafeForeignClass {
        public:
            CoChar(void* ptr) : SafeForeignClass(ptr) {}

            Coordinates3D GetVisualPosition() {
                // Only log memory access for successful reads to reduce spam
                if (!data()) {
                    return { 0, 0, 0 };
                }
                
                Coordinates3D result;
                if (!Debug::SafeRead<Coordinates3D>(data(), Offsets::CO_CHAR_VISUAL_POSITION, result)) {
                    // Reduce log spam for position failures
                    return { 0, 0, 0 };
                }
                
                return result;
            }
        };

        /**
         * @brief Agent wrapper for character entities
         */
        class AgChar : public SafeForeignClass {
        public:
            AgChar(void* ptr) : SafeForeignClass(ptr) {}

            CoChar GetCoChar() {
                LOG_MEMORY("AgChar", "GetCoChar", data(), Offsets::AG_CHAR_CO_CHAR);
                
                if (!data()) {
                    LOG_ERROR("AgChar::GetCoChar - AgChar data is null");
                    return CoChar(nullptr);
                }
                
                void* coCharPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x50, coCharPtr)) {
                    LOG_WARN("AgChar::GetCoChar - Failed to read CoChar pointer at offset 0x50");
                    return CoChar(nullptr);
                }
                
                LOG_PTR("CoChar", coCharPtr);
                return CoChar(coCharPtr);
            }

            uint32_t GetType() {
                LOG_MEMORY("AgChar", "GetType", data(), Offsets::AG_CHAR_TYPE);
                
                if (!data()) {
                    LOG_ERROR("AgChar::GetType - AgChar data is null");
                    return 0;
                }
                
                uint32_t type = 0;
                if (!Debug::SafeRead<uint32_t>(data(), Offsets::AG_CHAR_TYPE, type)) {
                    LOG_ERROR("AgChar::GetType - Failed to read type at offset 0x08");
                    return 0;
                }
                
                LOG_DEBUG("AgChar::GetType - Type: %u", type);
                return type;
            }
        };

    } // namespace ReClass
} // namespace kx