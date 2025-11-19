#pragma once
#include <cstdint>
#include <functional>

namespace kx {

struct CombatStateKey {
    uint32_t agentId;
    const void* address;

    CombatStateKey(uint32_t id, const void* addr) : agentId(id), address(addr) {}

    bool operator==(const CombatStateKey& other) const {
        if (agentId != 0 && other.agentId != 0) {
            return agentId == other.agentId;
        }
        return address == other.address;
    }
};

struct CombatStateKeyHash {
    std::size_t operator()(const CombatStateKey& k) const {
        if (k.agentId != 0) {
            return std::hash<uint32_t>{}(k.agentId);
        }
        return std::hash<const void*>{}(k.address);
    }
};

} // namespace kx

