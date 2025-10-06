#pragma once

#include <cstdint>

namespace kx {

/**
 * @brief Constants for memory scanning and address resolution
 * 
 * These constants define various offsets, sizes, and indices used in 
 * pattern scanning and RIP-relative address resolution.
 */
namespace AddressingConstants {
    // Instruction sizes for x64 architecture
    constexpr size_t RELATIVE_OFFSET_SIZE = 4;        // Size of relative offset in instructions
    constexpr size_t LEA_INSTRUCTION_SIZE = 7;        // Size of LEA instruction
    constexpr size_t CALL_INSTRUCTION_SIZE = 5;       // Size of CALL instruction
    constexpr size_t MOV_INSTRUCTION_SIZE = 7;        // Size of MOV instruction for RIP-relative addressing
    
    // Pattern search ranges
    constexpr size_t AGENT_ARRAY_SEARCH_RANGE = 0x300;  // Search range for LEA instruction in AvContext
    
    // Instruction offsets for parsing
    constexpr size_t LEA_OFFSET_POSITION = 3;         // Position of offset in LEA instruction
    constexpr size_t MOV_OFFSET_POSITION = 3;         // Position of offset in MOV instruction
    
    // Memory structure offsets
    constexpr size_t AGENT_ARRAY_OFFSET = 0x8;        // Offset from agent struct base to actual array
    
    // Pattern-specific offsets
    constexpr size_t BGFX_PATTERN_OFFSET = 0x35;      // Offset from pattern to function start
    constexpr size_t ALERT_CONTEXT_CALL_OFFSET = 0x19; // Offset from locator to call instruction
    
    // VTable indices
    constexpr size_t GAME_THREAD_UPDATE_VTABLE_INDEX = 0; // Index in VTable for GameThreadUpdate function
}

// A struct to hold all game-related pointers and addresses.
struct GamePointers {
    uintptr_t agentArray = 0;
    uintptr_t worldViewContextPtr = 0;
    uintptr_t bgfxContextFunc = 0;
    uintptr_t contextCollectionFunc = 0;
    uintptr_t gameThreadUpdateFunc = 0;
    uintptr_t decodeTextFunc = 0;
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
    static uintptr_t GetDecodeTextFunc() { return s_pointers.decodeTextFunc; }
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
    static void ScanDecodeTextFunc();

    // Single static struct instance holding all pointers.
    static GamePointers s_pointers;
};

} // namespace kx
