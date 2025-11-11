#pragma once

#include <Windows.h>
#include <ankerl/unordered_dense.h>
#include <chrono>
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

    // --- Pointer Cache for Performance ---
    static constexpr uint64_t CACHE_TTL = 5000; // ms
    static constexpr uint64_t CACHE_TTL_JITTER = 1000; // ms
    static constexpr uint64_t CACHE_CLEANUP_INTERVAL = 8000; // ms, this should be less than CACHE_TTL + CACHE_TTL_JITTER combined

    // Thread-safe cache accessors using function-local statics
    inline ankerl::unordered_dense::segmented_map<uintptr_t, uint64_t>& GetValidPagesCache() {
        thread_local ankerl::unordered_dense::segmented_map<uintptr_t, uint64_t> cache;
        return cache;
    }
    
    inline uint64_t& GetLastCacheCleanup() {
        thread_local uint64_t lastCleanup = GetTickCount64();
        return lastCleanup;
    }

    inline uint64_t GetRandomCacheTTL(uint64_t now) {
        thread_local std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<uint64_t> jitter(0, CACHE_TTL_JITTER);
        return now + CACHE_TTL + jitter(rng);
    }

    /**
     * @brief Removes expired addresses from the cache periodically
     */
    inline void CleanupCacheIfNeeded(uint64_t now) {
        uint64_t& lastCleanup = GetLastCacheCleanup();
        if (now - lastCleanup >= CACHE_CLEANUP_INTERVAL) {
            auto& cache = GetValidPagesCache();
            for (auto it = cache.begin(); it != cache.end(); ) {
                if (now > it->second) {
                    it = cache.erase(it);
                }
                else {
                    ++it;
                }
            }
            lastCleanup = now;
        }
    }

    inline uint64_t GetPageSize() {
        static uint64_t pageSize;
        if (!pageSize) {
            SYSTEM_INFO info = {};
            GetSystemInfo(&info);
            pageSize = info.dwPageSize;
        }
        return pageSize;
    }

    inline uintptr_t GetPageBase(uintptr_t addr) noexcept {
        return addr & ~(GetPageSize() - 1);
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
        auto now = GetTickCount64();
        CleanupCacheIfNeeded(now);
        
        // Check cache first - avoid expensive VirtualQuery if already validated
        // Use find() for single lookup instead of count() + insert()
        // Use the page base instead of the address, since memory protections apply to whole pages, meaning we need to cache fewer addresses
        uintptr_t page = GetPageBase(address);
        auto& validPages = GetValidPagesCache();
        auto it = validPages.find(page);
        bool entryCached = it != validPages.end();
        // Check if validPages already contains this page AND it has not expired yet
        if (entryCached && it->second > now) {
            return true;
        }
        
        MEMORY_BASIC_INFORMATION mbi = {};
        SIZE_T result = VirtualQuery(ptr, &mbi, sizeof(mbi));
        
        if (result == 0) return false;
        
        // Check if memory is committed and readable
        if (mbi.State != MEM_COMMIT) return false;
        if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) return false;
        if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
        
        // Cache the valid page for future use
        // If it was already cached, just refresh the expiry, if not, add it to the cache
        if (entryCached) {
            it->second = GetRandomCacheTTL(now);
        }
        else {
            validPages.emplace(page, GetRandomCacheTTL(now));
        }
        
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