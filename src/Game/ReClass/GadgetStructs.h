#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Core/AppState.h"
#include "../../Utils/SafeForeignClass.h"
#include "../Coordinates.h"
#include "../GameEnums.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Coordinate/Object wrapper for gadget positioning
         */
        class CoKeyFramed : public SafeForeignClass {
        public:
            CoKeyFramed(void* ptr) : SafeForeignClass(ptr) {}
            
            Coordinates3D GetPosition() {
                LOG_MEMORY("CoKeyFramed", "GetPosition", data(), 0x0030);
                
                if (!data()) {
                    LOG_ERROR("CoKeyFramed::GetPosition - CoKeyFramed data is null");
                    return Coordinates3D{ 0,0,0 };
                }
                
                Coordinates3D position;
                if (!Debug::SafeRead<Coordinates3D>(data(), 0x0030, position)) {
                    LOG_ERROR("CoKeyFramed::GetPosition - Failed to read position at offset 0x0030");
                    return Coordinates3D{ 0,0,0 };
                }
                
                LOG_DEBUG("CoKeyFramed::GetPosition - Position: (%.2f, %.2f, %.2f)", 
                         position.x, position.y, position.z);
                return position;
            }
        };

        /**
         * @brief Agent wrapper for gadget entities
         */
        class AgKeyFramed : public SafeForeignClass {
        public:
            AgKeyFramed(void* ptr) : SafeForeignClass(ptr) {}
            
            CoKeyFramed GetCoKeyFramed() {
                LOG_MEMORY("AgKeyFramed", "GetCoKeyFramed", data(), 0x0050);
                
                if (!data()) {
                    LOG_ERROR("AgKeyFramed::GetCoKeyFramed - AgKeyFramed data is null");
                    return CoKeyFramed(nullptr);
                }
                
                void* coKeyFramedPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x0050, coKeyFramedPtr)) {
                    LOG_ERROR("AgKeyFramed::GetCoKeyFramed - Failed to read CoKeyFramed pointer at offset 0x0050");
                    return CoKeyFramed(nullptr);
                }
                
                LOG_PTR("CoKeyFramed", coKeyFramedPtr);
                return CoKeyFramed(coKeyFramedPtr);
            }
        };

        /**
         * @brief Main gadget wrapper for interactive objects in the world
         */
        class GdCliGadget : public SafeForeignClass {
        public:
            GdCliGadget(void* ptr) : SafeForeignClass(ptr) {}

            Game::GadgetType GetGadgetType() {
                LOG_MEMORY("GdCliGadget", "GetGadgetType", data(), 0x0200);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::GetGadgetType - GdCliGadget data is null");
                    return Game::GadgetType::None;
                }
                
                uint32_t typeValue = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x0200, typeValue)) {
                    LOG_ERROR("GdCliGadget::GetGadgetType - Failed to read gadget type at offset 0x0200");
                    return Game::GadgetType::None;
                }
                
                Game::GadgetType gadgetType = static_cast<Game::GadgetType>(typeValue);
                LOG_DEBUG("GdCliGadget::GetGadgetType - Type: %u", static_cast<uint32_t>(gadgetType));
                return gadgetType;
            }

            // This is the key for filtering depleted resource nodes.
            // The flag 0x2 appears to mean "is active/gatherable".
            bool IsGatherable() {
                LOG_MEMORY("GdCliGadget", "IsGatherable", data(), 0x04E8);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::IsGatherable - GdCliGadget data is null");
                    return false;
                }
                
                uint32_t flags = 0;
                if (!Debug::SafeRead<uint32_t>(data(), 0x04E8, flags)) {
                    LOG_ERROR("GdCliGadget::IsGatherable - Failed to read flags at offset 0x04E8");
                    return false;
                }
                
                bool gatherable = (flags & 0x2) != 0;
                LOG_DEBUG("GdCliGadget::IsGatherable - Flags: 0x%X, Gatherable: %s", flags, gatherable ? "true" : "false");
                return gatherable;
            }

            AgKeyFramed GetAgKeyFramed() {
                LOG_MEMORY("GdCliGadget", "GetAgKeyFramed", data(), 0x0038);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::GetAgKeyFramed - GdCliGadget data is null");
                    return AgKeyFramed(nullptr);
                }
                
                void* agKeyFramedPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), 0x0038, agKeyFramedPtr)) {
                    LOG_ERROR("GdCliGadget::GetAgKeyFramed - Failed to read AgKeyFramed pointer at offset 0x0038");
                    return AgKeyFramed(nullptr);
                }
                
                LOG_PTR("AgKeyFramed", agKeyFramedPtr);
                return AgKeyFramed(agKeyFramedPtr);
            }
        };

    } // namespace ReClass
} // namespace kx