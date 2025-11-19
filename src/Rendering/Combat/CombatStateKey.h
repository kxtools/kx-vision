#pragma once
#include <cstdint>
#include <functional>
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
    std::size_t operator()(const CombatStateKey& k) const {
        if (k.agentId != 0) {
            return std::hash<uint32_t>{}(k.agentId);
        }
        return std::hash<const void*>{}(k.address);
    }
};

} // namespace kx

template <>
struct ankerl::unordered_dense::hash<kx::CombatStateKey> {
    using is_avalanching = void;

    [[nodiscard]] auto operator()(kx::CombatStateKey const& k) const noexcept -> uint64_t {
        if (k.agentId != 0) {
            return ankerl::unordered_dense::detail::wyhash::hash(k.agentId);
        }
        return ankerl::unordered_dense::detail::wyhash::hash(&k.address, sizeof(void*));
    }
};
