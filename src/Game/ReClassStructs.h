#pragma once

#include "../Utils/ForeignClass.h"
#include "GameStructs.h" // Re-use Coordinates3D

namespace kx {
    namespace ReClass {

        // Forward declarations
        class ChCliCharacter;
        class AgChar;

        // --- Safe Wrappers for Game Structures ---

        class CoChar : public ForeignClass {
        public:
            CoChar(void* ptr) : ForeignClass(ptr) {}

            // Offset from ReClass: CoChar->vec3PositionVisual (0x01A0)
            // This seems to be the character's visual position.
            Coordinates3D GetVisualPosition() {
                __try {
                    if (!data()) return { 0, 0, 0 };
                    return get<Coordinates3D>(0x30);

                    // backup offsets, might work too
                    //return get<Coordinates3D>(0x120);
                    //return get<Coordinates3D>(0x130);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return { 0, 0, 0 };
                }
            }
        };

        class AgChar : public ForeignClass {
        public:
            AgChar(void* ptr) : ForeignClass(ptr) {}

            // Offset from ReClass: AgChar->pCoChar (0x0050)
            CoChar GetCoChar() {
                __try {
                    if (!data()) return CoChar(nullptr);
                    return CoChar(get<void*>(0x50));
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return CoChar(nullptr);
                }
            }
        };

        class ChCliCharacter : public ForeignClass {
        public:
            ChCliCharacter(void* ptr) : ForeignClass(ptr) {}

            // Offset from ReClass: ChCliCharacter->pAgChar (0x0098)
            // THIS IS THE FUNCTION THAT WAS CRASHING. IT IS NOW PROTECTED.
            AgChar GetAgent() {
                __try {
                    if (!data()) return AgChar(nullptr);
                    return AgChar(get<void*>(0x98));
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return AgChar(nullptr);
                }
            }
        };

        class ChCliContext : public ForeignClass {
        public:
            ChCliContext(void* ptr) : ForeignClass(ptr) {}

            ChCliCharacter** GetCharacterList() {
                __try {
                    if (!data()) return nullptr;
                    return get<ChCliCharacter**>(0x60);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return nullptr;
                }
            }

            uint32_t GetCharacterListCapacity() {
                __try {
                    if (!data()) return 0;
                    return get<uint32_t>(0x68);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return 0;
                }
            }
        };

        class ContextCollection : public ForeignClass {
        public:
            ContextCollection(void* ptr) : ForeignClass(ptr) {}

            ChCliContext GetChCliContext() {
                __try {
                    if (!data()) return ChCliContext(nullptr);
                    return ChCliContext(get<void*>(0x98));
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return ChCliContext(nullptr);
                }
            }
        };


    } // namespace ReClass
} // namespace kx