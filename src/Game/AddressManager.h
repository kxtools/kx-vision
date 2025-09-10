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
    static uintptr_t GetContextCollectionFunc();

private:
    static void Scan();
    static void ScanAgentArray();
    static void ScanWorldViewContextPtr();
    static void ScanBgfxContextFunc();
    static void ScanContextCollectionFunc();

    static uintptr_t m_agentArray;
    static uintptr_t m_worldViewContextPtr;
    static uintptr_t m_bgfxContextFunc;
    static uintptr_t m_contextCollectionFunc;
};

} // namespace kx