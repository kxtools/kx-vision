#pragma once

#include <cstdint>

namespace kx { namespace Hooking { void __fastcall DetourGameThread(void* pInst, int frame_time); } }

namespace kx {

class AddressManager {
public:
    static void Initialize();
    static void Refresh();
    static uintptr_t GetAgentArray();
    static uintptr_t GetWorldViewContextPtr();
    static uintptr_t GetBgfxContextFunc();
    static uintptr_t GetContextCollectionFunc();
    static uintptr_t GetGameThreadUpdateFunc();

    static void* GetContextCollectionPtr();

private:
    // NEW: Friend our hook so it can access the private s_pContextCollection.
    friend void Hooking::DetourGameThread(void* pInst, int frame_time);

    static void Scan();
    static void ScanAgentArray();
    static void ScanWorldViewContextPtr();
    static void ScanBgfxContextFunc();
    static void ScanContextCollectionFunc();
    static void ScanGameThreadUpdateFunc();

    static uintptr_t m_agentArray;
    static uintptr_t m_worldViewContextPtr;
    static uintptr_t m_bgfxContextFunc;
    static uintptr_t m_contextCollectionFunc;
    static uintptr_t m_gameThreadUpdateFunc;

    static void* s_pContextCollection;
};

} // namespace kx