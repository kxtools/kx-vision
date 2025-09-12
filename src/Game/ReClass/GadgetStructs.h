#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Core/AppState.h"
#include "../../Utils/SafeForeignClass.h"
#include "../Coordinates.h"
#include "../GameEnums.h"
#include "../offsets.h"

namespace kx {
    namespace ReClass {

        /**
         * @brief Coordinate/Object wrapper for keyframed entities (gadgets)
         */
        class CoKeyFramed : public kx::SafeForeignClass {
        public:
            CoKeyFramed(void* ptr) : kx::SafeForeignClass(ptr) {}

            kx::Coordinates3D GetPosition() {
                LOG_MEMORY("CoKeyFramed", "GetPosition", data(), Offsets::CO_KEYFRAMED_POSITION);
                
                if (!data()) {
                    LOG_ERROR("CoKeyFramed::GetPosition - CoKeyFramed data is null");
                    return kx::Coordinates3D{ 0,0,0 };
                }
                
                kx::Coordinates3D position;
                if (!Debug::SafeRead<kx::Coordinates3D>(data(), Offsets::CO_KEYFRAMED_POSITION, position)) {
                    LOG_ERROR("CoKeyFramed::GetPosition - Failed to read position at offset CO_KEYFRAMED_POSITION");
                    return kx::Coordinates3D{ 0,0,0 };
                }
                
                LOG_DEBUG("CoKeyFramed::GetPosition - Position: (%.2f, %.2f, %.2f)", position.X, position.Y, position.Z);
                return position;
            }
        };

        /**
         * @brief Agent wrapper for keyframed entities
         */
        class AgKeyFramed : public kx::SafeForeignClass {
        public:
            AgKeyFramed(void* ptr) : kx::SafeForeignClass(ptr) {}

            CoKeyFramed GetCoKeyFramed() {
                LOG_MEMORY("AgKeyFramed", "GetCoKeyFramed", data(), Offsets::AG_KEYFRAMED_CO_KEYFRAMED);
                
                if (!data()) {
                    LOG_ERROR("AgKeyFramed::GetCoKeyFramed - AgKeyFramed data is null");
                    return CoKeyFramed(nullptr);
                }
                
                void* coKeyFramedPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), Offsets::AG_KEYFRAMED_CO_KEYFRAMED, coKeyFramedPtr)) {
                    LOG_ERROR("AgKeyFramed::GetCoKeyFramed - Failed to read CoKeyFramed pointer at offset AG_KEYFRAMED_CO_KEYFRAMED");
                    return CoKeyFramed(nullptr);
                }
                
                LOG_PTR("CoKeyFramed", coKeyFramedPtr);
                return CoKeyFramed(coKeyFramedPtr);
            }
        };

        /**
         * @brief Client gadget wrapper
         */
        class GdCliGadget : public kx::SafeForeignClass {
        public:
            GdCliGadget(void* ptr) : kx::SafeForeignClass(ptr) {}

            Game::GadgetType GetGadgetType() {
                LOG_MEMORY("GdCliGadget", "GetGadgetType", data(), Offsets::GD_CLI_GADGET_TYPE);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::GetGadgetType - GdCliGadget data is null");
                    return Game::GadgetType::None;
                }
                
                uint32_t typeValue = 0;
                if (!Debug::SafeRead<uint32_t>(data(), Offsets::GD_CLI_GADGET_TYPE, typeValue)) {
                    LOG_ERROR("GdCliGadget::GetGadgetType - Failed to read gadget type at offset GD_CLI_GADGET_TYPE");
                    return Game::GadgetType::None;
                }
                
                Game::GadgetType gadgetType = static_cast<Game::GadgetType>(typeValue);
                LOG_DEBUG("GdCliGadget::GetGadgetType - Type: %u", static_cast<uint32_t>(gadgetType));
                return gadgetType;
            }

            // The flag GADGET_FLAG_GATHERABLE appears to mean "is active/gatherable".
            bool IsGatherable() {
                LOG_MEMORY("GdCliGadget", "IsGatherable", data(), Offsets::GD_CLI_GADGET_FLAGS);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::IsGatherable - GdCliGadget data is null");
                    return false;
                }
                
                uint32_t flags = 0;
                if (!Debug::SafeRead<uint32_t>(data(), Offsets::GD_CLI_GADGET_FLAGS, flags)) {
                    LOG_ERROR("GdCliGadget::IsGatherable - Failed to read flags at offset GD_CLI_GADGET_FLAGS");
                    return false;
                }
                
                bool gatherable = (flags & Offsets::GADGET_FLAG_GATHERABLE) != 0;
                LOG_DEBUG("GdCliGadget::IsGatherable - Flags: 0x%X, Gatherable: %s", flags, gatherable ? "true" : "false");
                return gatherable;
            }

            AgKeyFramed GetAgKeyFramed() {
                LOG_MEMORY("GdCliGadget", "GetAgKeyFramed", data(), Offsets::GD_CLI_GADGET_AG_KEYFRAMED);
                
                if (!data()) {
                    LOG_ERROR("GdCliGadget::GetAgKeyFramed - GdCliGadget data is null");
                    return AgKeyFramed(nullptr);
                }
                
                void* agKeyFramedPtr = nullptr;
                if (!Debug::SafeRead<void*>(data(), Offsets::GD_CLI_GADGET_AG_KEYFRAMED, agKeyFramedPtr)) {
                    LOG_ERROR("GdCliGadget::GetAgKeyFramed - Failed to read AgKeyFramed pointer at offset GD_CLI_GADGET_AG_KEYFRAMED");
                    return AgKeyFramed(nullptr);
                }
                
                LOG_PTR("AgKeyFramed", agKeyFramedPtr);
                return AgKeyFramed(agKeyFramedPtr);
            }
        };

    } // namespace ReClass
} // namespace kx
