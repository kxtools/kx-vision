#pragma once

#include <cstdint>

namespace kx {

class AddressManager {
public:
    static void Initialize();
    static void Refresh();
    static uintptr_t GetAgentArray();
    static uintptr_t GetWorldViewContextPtr();
    static uintptr_t GetBgfxContextFunc();

private:
    static void Scan();
    static void ScanAgentArray();
    static void ScanWorldViewContextPtr();
    static void ScanBgfxContextFunc();
    static uintptr_t m_agentArray;
    static uintptr_t m_worldViewContextPtr;
    static uintptr_t m_bgfxContextFunc;
};

} // namespace kx