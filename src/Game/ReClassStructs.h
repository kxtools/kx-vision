#pragma once

#include "../Utils/ForeignClass.h"
#include "GameStructs.h" // Re-use Coordinates3D

namespace kx {
    namespace ReClass {

	    // Forward declarations
        class ChCliCharacter;
        class AgChar;
        class ChCliHealth;
        class ChCliCoreStats;
        class ChCliPlayer;
        class ChCliEnergies;
        class GdCliGadget;

        // --- Safe Wrappers for Game Structures ---

        class CoChar : public ForeignClass {
        public:
            CoChar(void* ptr) : ForeignClass(ptr) {}

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

            CoChar GetCoChar() {
                __try {
                    if (!data()) return CoChar(nullptr);
                    return CoChar(get<void*>(0x50));
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return CoChar(nullptr);
                }
            }

            uint32_t GetType() {
                __try {
                    return data() ? get<uint32_t>(0x08) : 0;
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    return 0;
                }
            }
        };

        class ChCliEnergies : public ForeignClass {
        public:
            ChCliEnergies(void* ptr) : ForeignClass(ptr) {}
            float GetCurrent() { __try { return data() ? get<float>(0x0C) : 0.0f; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0.0f; } }
            float GetMax() { __try { return data() ? get<float>(0x10) : 0.0f; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0.0f; } }
        };

        class ChCliHealth : public ForeignClass {
        public:
            ChCliHealth(void* ptr) : ForeignClass(ptr) {}
            float GetCurrent() { __try { return data() ? get<float>(0x0C) : 0.0f; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0.0f; } }
            float GetMax() { __try { return data() ? get<float>(0x10) : 0.0f; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0.0f; } }
        };

        class ChCliCoreStats : public ForeignClass {
        public:
            ChCliCoreStats(void* ptr) : ForeignClass(ptr) {}
            uint8_t GetRace() { __try { return data() ? get<uint8_t>(0x33) : 0; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; } }
            uint32_t GetLevel() { __try { return data() ? get<uint32_t>(0xAC) : 0; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; } }
            uint32_t GetProfession() { __try { return data() ? get<uint32_t>(0x12C) : 0; } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; } }
        };

        class ChCliCharacter : public ForeignClass {
        public:
            ChCliCharacter(void* ptr) : ForeignClass(ptr) {}

            AgChar GetAgent() {
                __try {
                    if (!data()) return AgChar(nullptr);
                    return AgChar(get<void*>(0x98));
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return AgChar(nullptr);
                }
            }

            ChCliHealth GetHealth() { 
                __try { return ChCliHealth(data() ? get<void*>(0x03E8) : nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { return ChCliHealth(nullptr); } 
            }

            ChCliCoreStats GetCoreStats() { 
                __try { return ChCliCoreStats(data() ? get<void*>(0x0388) : nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { return ChCliCoreStats(nullptr); } 
            }

            uint32_t GetAttitude() { 
                __try { return data() ? get<uint32_t>(0x00C0) : 1; } __except (EXCEPTION_EXECUTE_HANDLER) { return 1; } // Default to Neutral
            }

            ChCliEnergies GetEnergies() { 
                __try { return ChCliEnergies(data() ? get<void*>(0x03D8) : nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { return ChCliEnergies(nullptr); } 
            }
        };

        class ChCliPlayer : public ForeignClass {
        public:
            ChCliPlayer(void* ptr) : ForeignClass(ptr) {}
            ChCliCharacter GetCharacter() { __try { return ChCliCharacter(data() ? get<void*>(0x18) : nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { return ChCliCharacter(nullptr); } }
            const wchar_t* GetName() { __try { return data() ? get<wchar_t*>(0x68) : nullptr; } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; } }
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

            ChCliPlayer** GetPlayerList() {
                __try {
                    if (!data()) return nullptr;
                    return get<ChCliPlayer**>(0x80);
                } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
            }

            uint32_t GetPlayerListSize() {
                __try {
                    if (!data()) return 0;
                    return get<uint32_t>(0x88);
                } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
            }
        };

        class GdCliContext : public ForeignClass {
        public:
            GdCliContext(void* ptr) : ForeignClass(ptr) {}

            GdCliGadget** GetGadgetList() {
                __try {
                    if (!data()) return nullptr;
                    return get<GdCliGadget**>(0x0030); // Offset of pGadgetList
                }
                __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
            }

            uint32_t GetGadgetListCapacity() {
                __try {
                    if (!data()) return 0;
                    return get<uint32_t>(0x0038); // Offset of dwGadgetListCapacity
                }
                __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
            }
        };

        class CoKeyFramed : public ForeignClass {
        public:
            CoKeyFramed(void* ptr) : ForeignClass(ptr) {}
            Coordinates3D GetPosition() {
                __try { return data() ? get<Coordinates3D>(0x0030) : Coordinates3D{ 0,0,0 }; }
                __except (EXCEPTION_EXECUTE_HANDLER) { return Coordinates3D{ 0,0,0 }; }
            }
        };

        class AgKeyFramed : public ForeignClass {
        public:
            AgKeyFramed(void* ptr) : ForeignClass(ptr) {}
            CoKeyFramed GetCoKeyFramed() {
                __try { return CoKeyFramed(data() ? get<void*>(0x0050) : nullptr); }
                __except (EXCEPTION_EXECUTE_HANDLER) { return CoKeyFramed(nullptr); }
            }
        };

        class GdCliGadget : public ForeignClass {
        public:
            GdCliGadget(void* ptr) : ForeignClass(ptr) {}

            uint32_t GetGadgetType() {
                __try { return data() ? get<uint32_t>(0x0200) : 0; }
                __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
            }

            // This is the key for filtering depleted resource nodes.
            // The flag 0x2 appears to mean "is active/gatherable".
            bool IsGatherable() {
                __try { return data() ? (get<uint32_t>(0x04E8) & 0x2) : false; }
                __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
            }

            AgKeyFramed GetAgKeyFramed() {
                __try { return AgKeyFramed(data() ? get<void*>(0x0038) : nullptr); }
                __except (EXCEPTION_EXECUTE_HANDLER) { return AgKeyFramed(nullptr); }
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

            GdCliContext GetGdCliContext() {
                __try {
                    if (!data()) return GdCliContext(nullptr);
                    return GdCliContext(get<void*>(0x0138));
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return GdCliContext(nullptr);
                }
            }
        };


    } // namespace ReClass
} // namespace kx
