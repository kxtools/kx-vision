#pragma once

#include <windows.h>

#include <cstdint>
#include <cstring>
#include <limits>

#include "../Memory/AddressManager.h"

namespace kx {
namespace SafeAccess {

    constexpr uintptr_t MIN_VALID = 0x10000;
    constexpr uintptr_t MAX_VALID = 0x7FFFFFFEFFFF;
    constexpr uint32_t MAX_GAME_CONTAINER_CAPACITY = 1u << 20;

    inline bool IsAddressInBounds(const void* ptr) noexcept {
        const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        return address >= MIN_VALID && address <= MAX_VALID;
    }

    inline bool IsRangeInBounds(uintptr_t address, size_t size) noexcept {
        if (size == 0 || address < MIN_VALID || address > MAX_VALID) {
            return false;
        }

        const size_t tail = size - 1;
        return tail <= MAX_VALID - address;
    }

    template<typename T>
    inline bool TryRead(uintptr_t address, T& outValue) noexcept {
        if (!IsRangeInBounds(address, sizeof(T))) {
            return false;
        }

        // Crash-safety invariant: foreign reads get exactly one guarded typed access.
        // Do not add a separate probe: it adds hot-path cost and cannot prevent TOCTOU invalidation.
        __try {
            outValue = *reinterpret_cast<const T*>(address);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    template<typename T>
    inline bool TryRead(const void* address, T& outValue) noexcept {
        return TryRead(reinterpret_cast<uintptr_t>(address), outValue);
    }

    template<typename T>
    inline bool TryWrite(uintptr_t address, const T& value) noexcept {
        if (!IsRangeInBounds(address, sizeof(T))) {
            return false;
        }

        // Foreign writes must stay inside SEH; C++ catch (...) does not reliably catch access violations.
        __try {
            *reinterpret_cast<T*>(address) = value;
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline bool TryReadBuffer(uintptr_t address, void* outBuffer, size_t size) noexcept {
        if (!outBuffer || !IsRangeInBounds(address, size)) {
            return false;
        }

        // Keep copy itself guarded. A prior readability check can become stale before memcpy executes.
        __try {
            std::memcpy(outBuffer, reinterpret_cast<const void*>(address), size);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline bool ProbeReadableRange(uintptr_t address, size_t size) noexcept {
        if (!IsRangeInBounds(address, size)) {
            return false;
        }

        __try {
            const auto* bytes = reinterpret_cast<const volatile uint8_t*>(address);
            for (size_t i = 0; i < size; ++i) {
                (void)bytes[i];
            }
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline bool SafeReadVTable(void* object, uintptr_t& outVTable) noexcept {
        return TryRead(object, outVTable);
    }

    inline bool ProbeMemory(void* ptr) noexcept {
        uint8_t ignored = 0;
        return TryRead(ptr, ignored);
    }

    inline bool RawSafeReadByte(uintptr_t address, unsigned char& outByte) noexcept {
        return TryRead(address, outByte);
    }

    inline bool IsMemorySafe(void* ptr) noexcept {
        return ProbeMemory(ptr);
    }

    inline bool IsExecutableAddress(uintptr_t address) noexcept {
        if (!IsAddressInBounds(reinterpret_cast<const void*>(address))) {
            return false;
        }

        MEMORY_BASIC_INFORMATION info{};
        if (VirtualQuery(reinterpret_cast<const void*>(address), &info, sizeof(info)) != sizeof(info) ||
            info.State != MEM_COMMIT || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0) {
            return false;
        }

        const DWORD protection = info.Protect & 0xFF;
        return protection == PAGE_EXECUTE || protection == PAGE_EXECUTE_READ ||
               protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
    }

    inline bool IsValidGameObject(void* object) noexcept {
        if (!IsAddressInBounds(object)) {
            return false;
        }

        uintptr_t vtable = 0;
        if (!TryRead(object, vtable)) {
            return false;
        }

        const uintptr_t moduleBase = AddressManager::GetModuleBase();
        const size_t moduleSize = AddressManager::GetModuleSize();
        if (moduleBase < MIN_VALID || moduleBase > MAX_VALID || moduleSize == 0 ||
            moduleSize > MAX_VALID - moduleBase) {
            return false;
        }

        return vtable >= moduleBase && vtable < moduleBase + moduleSize;
    }

    inline bool IsVTablePointerValid(void* object) noexcept {
        return IsValidGameObject(object);
    }

    inline bool IsAgentIdSane(int32_t id) noexcept {
        return id > 0 && id < 0x0FFFFFFF;
    }

} // namespace SafeAccess
} // namespace kx
