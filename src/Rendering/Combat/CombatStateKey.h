#pragma once
#include <cstdint>
#include <ankerl/unordered_dense.h>

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
    using is_avalanching = void;

    [[nodiscard]] uint64_t operator()(const CombatStateKey& k) const noexcept {
        if (k.agentId != 0) {
            return ankerl::unordered_dense::detail::wyhash::hash(k.agentId);
        }
        return ankerl::unordered_dense::detail::wyhash::hash(&k.address, sizeof(void*));
    }
};

} // namespace kx

