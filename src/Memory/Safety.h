#pragma once

#include "../Memory/AddressManager.h"

namespace kx {
namespace SafeAccess {

    // Hard Limits (User Mode Address Space)
    constexpr uintptr_t MIN_VALID = 0x10000;      // Skip null & low partition
    constexpr uintptr_t MAX_VALID = 0x7FFFFFFEFFFF; // User mode limit

    /**
     * @brief Super-fast check. Does NOT guarantee readability, only sanity.
     */
    inline bool IsAddressInBounds(void* ptr) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        return addr >= MIN_VALID && addr <= MAX_VALID;
    }

    // ---------------------------------------------------------
    // HELPER 1: Safe Pointer Read
    // Isolated in its own function to avoid C2712 errors.
    // ---------------------------------------------------------
    inline bool SafeReadVTable(void* pObject, uintptr_t& outVTable) {
        __try {
            outVTable = *reinterpret_cast<uintptr_t*>(pObject);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    // ---------------------------------------------------------
    // HELPER 2: Memory Probe
    // Isolated in its own function to avoid C2712 errors.
    // ---------------------------------------------------------
    inline bool ProbeMemory(void* ptr) {
        __try {
            volatile char c = *reinterpret_cast<volatile char*>(ptr);
            (void)c;
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    // ---------------------------------------------------------
    // Main Safety Checks
    // ---------------------------------------------------------

    /**
     * @brief The High-Performance Safety Check.
     * RELIES ON /EHa (Async Exceptions) to catch crashes.
     */
    inline bool IsMemorySafe(void* ptr) {
        if (!IsAddressInBounds(ptr)) return false;
        return ProbeMemory(ptr);
    }

    /**
     * @brief Checks if an object is a Valid Game Object.
     * This is the BEST check for Entity Lists.
     */
    inline bool IsValidGameObject(void* pObject) {
        // 1. Fast sanity checks
        if (!IsAddressInBounds(pObject)) return false;

        // 2. Get Module Info
        // Note: We moved the static vars out or rely on fast getters
        // AddressManager::GetModuleBase() should be a simple return of a static var
        uintptr_t modBase = AddressManager::GetModuleBase();
        size_t modSize = AddressManager::GetModuleSize();

        // 3. Use the Helper for the dangerous read
        uintptr_t vtable = 0;
        if (SafeReadVTable(pObject, vtable)) {
            // 4. Validate VTable range
            if (modBase > 0 && modSize > 0) {
                return (vtable >= modBase && vtable < (modBase + modSize));
            }
        }

        return false;
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

        // Safely read the first pointer-sized value (VTable pointer) using helper
        uintptr_t vtablePtr = 0;
        if (!SafeReadVTable(pObject, vtablePtr)) {
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
