#pragma once

#include <ankerl/unordered_dense.h>
#include <cstdint>

namespace kx {

struct CombatObservationKey {
    uint32_t agentId;
    const void* address;

    CombatObservationKey(uint32_t id, const void* addr) : agentId(id), address(addr) {}

    [[nodiscard]] bool IsValid() const noexcept {
        return agentId != 0 || address != nullptr;
    }

    bool operator==(const CombatObservationKey& other) const noexcept {
        // Guardrail: key kind is part of identity. Mixing agent-ID and address keys
        // would violate unordered-map equality/hash requirements.
        const bool usesAgentId = agentId != 0;
        if (usesAgentId != (other.agentId != 0)) {
            return false;
        }
        return usesAgentId ? agentId == other.agentId : address == other.address;
    }
};

struct CombatObservationKeyHash {
    using is_avalanching = void;

    [[nodiscard]] uint64_t operator()(const CombatObservationKey& key) const noexcept {
        if (key.agentId != 0) {
            return ankerl::unordered_dense::detail::wyhash::hash(key.agentId);
        }
        return ankerl::unordered_dense::detail::wyhash::hash(&key.address, sizeof(void*));
    }
};

} // namespace kx
