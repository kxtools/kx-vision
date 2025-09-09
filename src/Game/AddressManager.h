#pragma once

#include <cstdint>

namespace kx {

class AddressManager {
public:
    static void Initialize();
    static void Refresh();
    static uintptr_t GetAgentArray();
    static uintptr_t GetWorldViewContextPtr();

private:
    static void Scan();
    static void ScanAgentArray();
    static void ScanWorldViewContextPtr();
    static uintptr_t m_agentArray;
    static uintptr_t m_worldViewContextPtr;
};

} // namespace kx