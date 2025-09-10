#pragma once

#include <cstdint>
#include <windows.h> // Required for EXCEPTION_EXECUTE_HANDLER

#include "ForeignClass.h"
#include "offsets.h"

namespace kx {

struct Coordinates3D {
    float X;
    float Y;
    float Z;
};

class Agent {
private:
    // This pointer is to a CAvAgent, the wrapper object in the agent array.
    kx::ForeignClass pAvAgent;

    // Helper to get the pAgentBase from the pAvAgent.
    // The pointer chain is confirmed by the C# reference code.
    kx::ForeignClass GetBaseAgent() {
        const uintptr_t MIN_VALID_POINTER = 0x10000;
        if (!pAvAgent || (uintptr_t)pAvAgent.data() < MIN_VALID_POINTER) return ForeignClass(nullptr);

        kx::ForeignClass p1 = pAvAgent.get<void*>(Offsets::AGENT_PTR_CHAIN_1);
        if (!p1 || (uintptr_t)p1.data() < MIN_VALID_POINTER) return ForeignClass(nullptr);

        return p1.get<void*>(Offsets::AGENT_PTR_CHAIN_2);
    }

public:
    Agent(void* ptr) : pAvAgent(ptr) {}

    Coordinates3D GetPosition() {
        Coordinates3D pos = { 0, 0, 0 };
        const uintptr_t MIN_VALID_POINTER = 0x10000;
        __try {
            kx::ForeignClass pAgentBase = GetBaseAgent();
            if (!pAgentBase) return pos;

            kx::ForeignClass pAgentTransform = pAgentBase.get<void*>(Offsets::AGENT_BASE_TRANSFORM);
            if (!pAgentTransform || (uintptr_t)pAgentTransform.data() < MIN_VALID_POINTER) return pos;

            pos.X = pAgentTransform.get<float>(Offsets::AGENT_TRANSFORM_X);
            pos.Y = pAgentTransform.get<float>(Offsets::AGENT_TRANSFORM_Y);
            pos.Z = pAgentTransform.get<float>(Offsets::AGENT_TRANSFORM_Z);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return { 0, 0, 0 };
        }
        return pos;
    }

    uint32_t GetId() {
        __try {
            kx::ForeignClass pAgentBase = GetBaseAgent();
            if (!pAgentBase) return 0;
            return pAgentBase.get<uint32_t>(Offsets::AGENT_BASE_ID);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return 0;
        }
    }

    // We don't have the AgentType enum defined yet, so we'll return an int
    int GetType() {
        __try {
            kx::ForeignClass pAgentBase = GetBaseAgent();
            if (!pAgentBase) return -1; // Return -1 for error
            return pAgentBase.get<int>(Offsets::AGENT_BASE_TYPE);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return -1;
        }
    }

    // We don't have the GadgetType enum, so we'll return an int
    int GetGadgetType() {
        __try {
            kx::ForeignClass pAgentBase = GetBaseAgent();
            if (!pAgentBase) return -1; // Return -1 for error
            return pAgentBase.get<int>(Offsets::AGENT_BASE_GADGET_TYPE);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return -1;
        }
    }

    explicit operator bool() const { return pAvAgent.data() != nullptr; }

	const void* GetAddress() const { return pAvAgent.data(); }
};

class AgentArray {
public:
    AgentArray(void* ptr_to_ptr) : ppArray(ptr_to_ptr) {}

    Agent GetAgent(size_t index) {
        if (!ppArray) return Agent(nullptr);
        void* pArray = ppArray.get<void*>(Offsets::AGENT_ARRAY_POINTER);
        if (!pArray) return Agent(nullptr);
        kx::ForeignClass arrayWrapper(pArray);
        return Agent(arrayWrapper.get<void*>(index * sizeof(void*)));
    }

    uint32_t Capacity() {
        if (!ppArray) return 0;
        return ppArray.get<uint32_t>(Offsets::AGENT_ARRAY_CAPACITY);
    }

    uint32_t Count() {
        if (!ppArray) return 0;
        return ppArray.get<uint32_t>(Offsets::AGENT_ARRAY_COUNT);
    }

private:
    kx::ForeignClass ppArray;
};

} // namespace kx