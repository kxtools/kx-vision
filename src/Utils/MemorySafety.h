#pragma once

#include <Windows.h>
#include <unordered_set>
#include <chrono>
#include "../Game/AddressManager.h"

namespace kx {
namespace SafeAccess {

    // --- Memory Safety Constants ---
    constexpr uintptr_t MIN_VALID_MEMORY_ADDRESS = 0x1000;      // Minimum valid user-mode address
    constexpr uintptr_t MAX_VALID_MEMORY_ADDRESS = 0x7FFFFFFFFFFF; // Maximum valid user-mode address on x64
    
    // --- Sanity Check Limits ---
    constexpr uint32_t MAX_REASONABLE_PLAYER_COUNT = 2000;      // Maximum expected players in a game instance
    constexpr uint32_t MAX_REASONABLE_CHARACTER_COUNT = 0x10000; // Maximum expected characters in memory
    constexpr uint32_t MAX_REASONABLE_GADGET_COUNT = 0x10000;   // Maximum expected gadgets in memory

    // --- Pointer Cache for Performance ---
    // Thread-safe cache accessors using function-local statics
    inline std::unordered_set<uintptr_t>& GetValidPointersCache() {
        thread_local std::unordered_set<uintptr_t> cache;
        return cache;
    }
    
    inline std::chrono::steady_clock::time_point& GetLastCacheClear() {
        thread_local std::chrono::steady_clock::time_point lastClear = std::chrono::steady_clock::now();
        return lastClear;
    }
    
    static constexpr auto CACHE_CLEAR_INTERVAL = std::chrono::seconds(5);

    /**
     * @brief Clears the pointer cache periodically to handle entity despawning
     */
    inline void ClearCacheIfNeeded() {
        auto now = std::chrono::steady_clock::now();
        auto& lastClear = GetLastCacheClear();
        if (now - lastClear >= CACHE_CLEAR_INTERVAL) {
            GetValidPointersCache().clear();
            lastClear = now;
        }
    }

    /**
     * @brief Validates if a memory address is safe to read (with caching for performance)
     * @param ptr Pointer to validate
     * @param size Size of data to read (default: pointer size)
     * @return true if memory is safe to read, false otherwise
     */
    inline bool IsMemorySafe(void* ptr, size_t size = sizeof(void*)) {
        if (!ptr) return false;
        
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        
        // Quick validation for obviously invalid addresses
        if (address < MIN_VALID_MEMORY_ADDRESS || address > MAX_VALID_MEMORY_ADDRESS) {
            return false;
        }

        // Clear cache periodically to handle entity despawning
        ClearCacheIfNeeded();
        
        // Check cache first - avoid expensive VirtualQuery if already validated
        // Use find() for single lookup instead of count() + insert()
        auto& validPointers = GetValidPointersCache();
        auto it = validPointers.find(address);
        if (it != validPointers.end()) {
            return true;
        }
        
        MEMORY_BASIC_INFORMATION mbi = {};
        SIZE_T result = VirtualQuery(ptr, &mbi, sizeof(mbi));
        
        if (result == 0) return false;
        
        // Check if memory is committed and readable
        if (mbi.State != MEM_COMMIT) return false;
        if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) return false;
        if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
        
        // Cache the valid pointer for future use
        validPointers.emplace(address);
        
        return true;
    }

    /**
     * @brief Validates if an object's VTable pointer resides within the game module
     * @param pObject Pointer to the object whose VTable should be validated
     * @return true if the VTable pointer is valid and within module bounds, false otherwise
     */
    inline bool IsVTablePointerValid(void* pObject) {
        if (!pObject || !IsMemorySafe(pObject)) {
            return false;
        }

        // Safely read the first pointer-sized value (VTable pointer)
        uintptr_t vtablePtr = 0;
        __try {
            vtablePtr = *reinterpret_cast<uintptr_t*>(pObject);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }

        // Get cached module information from AddressManager
        uintptr_t moduleBase = kx::AddressManager::GetModuleBase();
        size_t moduleSize = kx::AddressManager::GetModuleSize();
        
        // If module information hasn't been scanned yet, can't validate
        if (moduleBase == 0 || moduleSize == 0) {
            return false;
        }

        // Check if VTable pointer is within the game module bounds
        return (vtablePtr >= moduleBase && vtablePtr < (moduleBase + moduleSize));
    }

} // namespace SafeAccess
} // namespace kx