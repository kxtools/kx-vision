#pragma once

#include <cstdint>

#include "ForeignClass.h"

namespace kx {

struct Coordinates3D {
    float X;
    float Y;
    float Z;
};

class Agent {
public:
    Agent(void* ptr) : pAgent(ptr) {}

    Coordinates3D GetPosition() {
        Coordinates3D pos = { 0, 0, 0 };
        const uintptr_t MIN_VALID_POINTER = 0x10000; // Define a threshold for valid pointers

        __try {
            if (!pAgent || (uintptr_t)pAgent.data() < MIN_VALID_POINTER) return pos;

            // Pointer chain for agent position
            kx::ForeignClass pPAgent = pAgent.get<void*>(0xC8);
            if (!pPAgent || (uintptr_t)pPAgent.data() < MIN_VALID_POINTER) return pos;
            pPAgent = pPAgent.get<void*>(0x38);
            if (!pPAgent || (uintptr_t)pPAgent.data() < MIN_VALID_POINTER) return pos;

            kx::ForeignClass pAgentTransform = pPAgent.get<void*>(0x50);
            if (!pAgentTransform || (uintptr_t)pAgentTransform.data() < MIN_VALID_POINTER) return pos;

            pos.X = pAgentTransform.get<float>(0x30);
            pos.Y = pAgentTransform.get<float>(0x34);
            pos.Z = pAgentTransform.get<float>(0x38);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            // On exception, return {0,0,0}
            return { 0, 0, 0 };
        }
        return pos;
    }

    explicit operator bool() const { return pAgent.data() != nullptr; }

private:
    kx::ForeignClass pAgent;
};

class AgentArray {
public:
    AgentArray(void* ptr_to_ptr) : ppArray(ptr_to_ptr) {}

    Agent GetAgent(size_t index) {
        if (!ppArray) return Agent(nullptr);
        void* pArray = ppArray.get<void*>(0x0);
        if (!pArray) return Agent(nullptr);
        kx::ForeignClass arrayWrapper(pArray);
        return Agent(arrayWrapper.get<void*>(index * sizeof(void*)));
    }

    uint32_t Count() {
        if (!ppArray) return 0;
        return ppArray.get<uint32_t>(0xC);
    }

private:
    kx::ForeignClass ppArray;
};

} // namespace kx