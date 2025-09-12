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
    
    // Module information for VTable validation
    uintptr_t moduleBase = 0;
    size_t moduleSize = 0;
};

class AddressManager {
public:
    static void Initialize();

    // Public setter for the hook to store the captured pointer.
    static void SetContextCollectionPtr(void* ptr);

    // Inlined getters for direct and fast access.
    static uintptr_t GetAgentArray() { return s_pointers.agentArray; }
    static uintptr_t GetWorldViewContextPtr() { return s_pointers.worldViewContextPtr; }
    static uintptr_t GetBgfxContextFunc() { return s_pointers.bgfxContextFunc; }
    static uintptr_t GetContextCollectionFunc() { return s_pointers.contextCollectionFunc; }
    static uintptr_t GetGameThreadUpdateFunc() { return s_pointers.gameThreadUpdateFunc; }
    static void* GetContextCollectionPtr() { return s_pointers.pContextCollection; }
    
    // Module information getters for VTable validation
    static uintptr_t GetModuleBase() { return s_pointers.moduleBase; }
    static size_t GetModuleSize() { return s_pointers.moduleSize; }

    // Local player detection
    static void* GetLocalPlayer();

private:
    static void* GetLocalPlayerImpl(void* pContextCollection); // Helper to avoid object unwinding issues
    static void Scan();
    static void ScanModuleInformation();
    static void ScanAgentArray();
    static void ScanWorldViewContextPtr();
    static void ScanBgfxContextFunc();
    static void ScanContextCollectionFunc();
    static void ScanGameThreadUpdateFunc();

    // Single static struct instance holding all pointers.
    static GamePointers s_pointers;
};

} // namespace kx
