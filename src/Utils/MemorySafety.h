#pragma once

#include <Windows.h>
#include <ankerl/unordered_dense.h>
#include <random>
#include "../Game/AddressManager.h"

namespace kx {
namespace SafeAccess {

    // --- Memory Safety Constants ---
    constexpr uintptr_t MIN_VALID_MEMORY_ADDRESS = 0x1000;      // Minimum valid user-mode address
    constexpr uintptr_t MAX_VALID_MEMORY_ADDRESS = 0x7FFFFFFFFFFF; // Maximum valid user-mode address on x64
    
    // --- Sanity Check Limits ---
    // These limits are based on actual observed game data and provide safety bounds
    // for iterator validation to detect obviously corrupted memory/capacity values
    constexpr uint32_t MAX_REASONABLE_PLAYER_COUNT = 1000;       // Observed ~134, set generous upper bound
    constexpr uint32_t MAX_REASONABLE_CHARACTER_COUNT = 15000;   // Observed ~9728, allow for larger instances  
    constexpr uint32_t MAX_REASONABLE_GADGET_COUNT = 15000;      // Observed ~9216, allow for resource-rich areas
    constexpr uint32_t MAX_REASONABLE_ATTACK_TARGET_COUNT = 5000; // Reasonable limit for attack targets
    constexpr uint32_t MAX_REASONABLE_ITEM_COUNT = 50000;         // Items lists can be large

    // --- Pointer Cache for Performance ---
    static constexpr uint64_t CACHE_TTL = 5000; // ms
    static constexpr uint64_t CACHE_TTL_JITTER = 1000; // ms
    static constexpr uint64_t CACHE_CLEANUP_INTERVAL = 8000; // ms, this should be greater than CACHE_TTL + CACHE_TTL_JITTER combined

    // --- Cache Structure ---
    struct PageCacheEntry {
        uint64_t expiry;
        bool isReadable;
    };

    // Thread-safe cache accessors using function-local statics
    inline ankerl::unordered_dense::map<uintptr_t, PageCacheEntry>& GetValidPagesCache() {
        thread_local ankerl::unordered_dense::map<uintptr_t, PageCacheEntry> cache;
        return cache;
    }
    
    inline uint64_t& GetLastCacheCleanup() {
        thread_local uint64_t lastCleanup = GetTickCount64();
        return lastCleanup;
    }

    inline uint64_t GetRandomCacheTTL(uint64_t now) {
        thread_local std::mt19937 rng{ std::random_device{}() };
        thread_local std::uniform_int_distribution<uint64_t> jitter(0, CACHE_TTL_JITTER);
        return now + CACHE_TTL + jitter(rng);
    }

    /**
     * @brief Removes expired pages from the cache periodically
     */
    inline void CleanupCacheIfNeeded(uint64_t now) {
        uint64_t& lastCleanup = GetLastCacheCleanup();
        if (now - lastCleanup >= CACHE_CLEANUP_INTERVAL) {
            auto& cache = GetValidPagesCache();
            for (auto it = cache.begin(); it != cache.end(); ) {
                if (now > it->second.expiry) {
                    it = cache.erase(it);
                }
                else {
                    ++it;
                }
            }
            lastCleanup = now;
        }
    }

    // Helper to check if a value is a power of two
    inline bool IsPowerOfTwo(uint64_t x) {
        return x != 0 && (x & (x - 1)) == 0;
    }
    
    inline uint64_t GetPageSize() {
        static uint64_t pageSize = 0;
        if (!pageSize) {
            SYSTEM_INFO info = {};
            GetSystemInfo(&info);
            // Validate page size: must be non-zero and power of two
            if (info.dwPageSize != 0 && IsPowerOfTwo(info.dwPageSize)) {
                pageSize = info.dwPageSize;
            } else {
                // Fallback: use 4096 (common page size)
                pageSize = 4096;
            }
        }
        return pageSize;
    }
    
    inline uintptr_t GetPageBase(uintptr_t addr) noexcept {
        return addr & ~(GetPageSize() - 1);
    }

    /**
     * @brief Validates if a memory address is safe to read.
     * OPTIMIZED: Uses a thread_local micro-cache to skip hash lookups for sequential reads.
     */
    inline bool IsMemorySafe(void* ptr, size_t size = sizeof(void*)) {
        if (!ptr) return false;
        
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        
        // 1. Fast Bounds Check (Constants defined at top of file)
        if (address < MIN_VALID_MEMORY_ADDRESS || address > MAX_VALID_MEMORY_ADDRESS) {
            return false;
        }

        // 2. Calculate Page Base (Assume 4KB pages for speed: 0xFFF)
        // This avoids the function call overhead of GetPageBase/GetPageSize in the hot path
        uintptr_t page = address & ~0xFFF;

        // 3. HOT PATH: Micro-Cache
        // If we are reading the same page as the immediate last call, return result instantly.
        // NO time checks, NO map lookups.
        static thread_local uintptr_t s_LastPage = 0;
        static thread_local bool s_LastResult = false;

        if (page == s_LastPage) {
            return s_LastResult;
        }

        // 4. COLD PATH: New Page Encountered
        s_LastPage = page;
        
        // Check the main hash map
        auto& cache = GetValidPagesCache();
        auto it = cache.find(page);
        
        // If found in map, trust it. We don't check expiry here to save CPU.
        // Expiry is handled by CleanupCacheIfNeeded() called elsewhere.
        if (it != cache.end()) {
            s_LastResult = it->second.isReadable;
            return s_LastResult;
        }

        // 5. SLOW PATH: System Call (VirtualQuery)
        // This only happens once per page per cache duration
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(ptr, &mbi, sizeof(mbi))) {
            s_LastResult = (mbi.State == MEM_COMMIT) && 
                           (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) &&
                           !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS));
        } else {
            s_LastResult = false;
        }

        // Update main cache
        // We only pay the cost of GetTickCount64 here (rarely)
        cache.insert_or_assign(page, PageCacheEntry{ GetRandomCacheTTL(GetTickCount64()), s_LastResult });
        
        return s_LastResult;
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
        uintptr_t moduleBase = AddressManager::GetModuleBase();
        size_t moduleSize = AddressManager::GetModuleSize();
        
        // If module information hasn't been scanned yet, can't validate
        if (moduleBase == 0 || moduleSize == 0) {
            return false;
        }

        // Check if VTable pointer is within the game module bounds
        return (vtablePtr >= moduleBase && vtablePtr < (moduleBase + moduleSize));
    }

} // namespace SafeAccess
} // namespace kx