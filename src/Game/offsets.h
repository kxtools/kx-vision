#pragma once
#include <cstdint>

namespace Offsets {
    // WvContext Offsets
    constexpr uintptr_t WV_CONTEXT_STATUS = 0x58;
    constexpr uintptr_t WV_CONTEXT_PTR_TO_RENDERER = 0x78;

    // Agent Pointer Chain
    constexpr uintptr_t AGENT_PTR_CHAIN_1 = 0xC8;
    constexpr uintptr_t AGENT_PTR_CHAIN_2 = 0x38;

    // AgentBase Members
    constexpr uintptr_t AGENT_BASE_TYPE = 0x8;
    constexpr uintptr_t AGENT_BASE_ID = 0xC;
    constexpr uintptr_t AGENT_BASE_GADGET_TYPE = 0x40;
    constexpr uintptr_t AGENT_BASE_TRANSFORM = 0x50;

    // AgentTransform Members
    constexpr uintptr_t AGENT_TRANSFORM_X = 0x30;
    constexpr uintptr_t AGENT_TRANSFORM_Y = 0x34;
    constexpr uintptr_t AGENT_TRANSFORM_Z = 0x38;

    // AgentArray Members
    constexpr uintptr_t AGENT_ARRAY_POINTER = 0x0;
    constexpr uintptr_t AGENT_ARRAY_CAPACITY = 0x8;
    constexpr uintptr_t AGENT_ARRAY_COUNT = 0xC;
}
