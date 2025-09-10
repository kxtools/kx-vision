#pragma once

#include <cstdint>

namespace kx {

// A struct to hold all game-related pointers and addresses.
struct GamePointers {
    uintptr_t agentArray = 0;
    uintptr_t worldViewContextPtr = 0;
    uintptr_t bgfxContextFunc = 0;
    uintptr_t contextCollectionFunc = 0;
    uintptr_t gameThreadUpdateFunc = 0;
    void* pContextCollection = nullptr;
};

class AddressManager {
public:
    static void Initialize();
    static void Refresh();

    // Public setter for the hook to store the captured pointer.
    static void SetContextCollectionPtr(void* ptr);

    // Inlined getters for direct and fast access.
    static uintptr_t GetAgentArray() { return s_pointers.agentArray; }
    static uintptr_t GetWorldViewContextPtr() { return s_pointers.worldViewContextPtr; }
    static uintptr_t GetBgfxContextFunc() { return s_pointers.bgfxContextFunc; }
    static uintptr_t GetContextCollectionFunc() { return s_pointers.contextCollectionFunc; }
    static uintptr_t GetGameThreadUpdateFunc() { return s_pointers.gameThreadUpdateFunc; }
    static void* GetContextCollectionPtr() { return s_pointers.pContextCollection; }

private:
    static void Scan();
    static void ScanAgentArray();
    static void ScanWorldViewContextPtr();
    static void ScanBgfxContextFunc();
    static void ScanContextCollectionFunc();
    static void ScanGameThreadUpdateFunc();

    // Single static struct instance holding all pointers.
    static GamePointers s_pointers;
};

} // namespace kx
