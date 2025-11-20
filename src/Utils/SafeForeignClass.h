#pragma once

#include <windows.h> // Required for VirtualProtect
#include <cstdint>   // Required for UINTPTR_MAX
#include "MemorySafety.h"
#include "DebugLogger.h"

namespace kx {

    namespace SafeForeignClassLimits {
        // Critical validation limits to prevent obvious attacks/corruption
        static constexpr uintptr_t MAX_REASONABLE_OFFSET = 0x100000;       // 1MB max offset
        static constexpr size_t MAX_REASONABLE_SIZE = 0x10000;             // 64KB max single access
        static constexpr uintptr_t MAX_REASONABLE_VTABLE_OFFSET = 0x1000;  // 4KB max vtable
        
        // Memory range validation (x64 user space)
        static constexpr uintptr_t MIN_FUNCTION_ADDRESS = 0x400000;        // Typical exe base
        static constexpr uintptr_t MAX_FUNCTION_ADDRESS = 0x7FF000000000;  // x64 user space limit
        static constexpr uintptr_t MIN_VTABLE_ADDRESS = 0x400000;          // Typical exe/dll base
        static constexpr uintptr_t MAX_VTABLE_ADDRESS = 0x7FF000000000;    // x64 user space limit
    }

    /**
     * @brief Memory-safe version of ForeignClass that validates all memory access
     * 
     * This class provides safe access to foreign memory layouts with proper validation
     * to prevent crashes from invalid memory access, following the patterns established
     * in our ESP rendering pipeline.
     * 
     * Key safety features:
     * - Construction-time validation with automatic nullification of unsafe pointers
     * - Range validation (1MB max offset, 64KB max access size)
     * - Integer overflow protection in pointer arithmetic
     * - VTable integrity validation with reasonable memory range checks
     * - Function pointer validation for executable memory ranges
     * - Exception handling for all memory operations
     * - Comprehensive validation methods for different risk levels
     */
    class SafeForeignClass {
    public:
        SafeForeignClass(void* ptr) : m_ptr(ptr) {
            // Critical validation on construction - nullify unsafe pointers immediately
            if (ptr && !SafeAccess::IsMemorySafe(ptr)) {
                m_ptr = nullptr; // Nullify unsafe pointers to prevent future crashes
            }
        }

        // Copy constructor with validation
        SafeForeignClass(const SafeForeignClass& other) : m_ptr(other.m_ptr) {
            // Re-validate on copy to ensure safety
            if (m_ptr && !SafeAccess::IsMemorySafe(m_ptr)) {
                m_ptr = nullptr;
            }
        }

        // Assignment operator with validation
        SafeForeignClass& operator=(const SafeForeignClass& other) {
            if (this != &other) {
                m_ptr = other.m_ptr;
                // Re-validate on assignment
                if (m_ptr && !SafeAccess::IsMemorySafe(m_ptr)) {
                    m_ptr = nullptr;
                }
            }
            return *this;
        }

        // Move constructor (safe since we validate on access)
        SafeForeignClass(SafeForeignClass&& other) noexcept : m_ptr(other.m_ptr) {
            other.m_ptr = nullptr;
        }

        // Move assignment
        SafeForeignClass& operator=(SafeForeignClass&& other) noexcept {
            if (this != &other) {
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        /**
         * @brief Read a member variable from foreign memory with comprehensive validation
         * @tparam T Type to read
         * @param offset Offset from base pointer
         * @param defaultValue Value to return if read fails (defaults to default-constructed T)
         * @return Value of type T, or defaultValue if memory is unsafe
         */
        template<typename T>
        [[nodiscard]] T ReadMember(uintptr_t offset, const T& defaultValue = T{}) const {
            if (!data()) {
                return defaultValue;
            }
            
            T result;
            if (!Debug::SafeRead<T>(data(), offset, result)) {
                return defaultValue;
            }
            
            return result;
        }

        /**
         * @brief FAST read. No VirtualQuery check. Use only after validating base pointer and inside guarded (__try/__except) regions.
         */
        template<typename T>
        [[nodiscard]] T ReadMemberFast(uintptr_t offset, const T& defaultValue = T{}) const {
            if (!m_ptr) {
                return defaultValue;
            }

            auto* address = reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(m_ptr) + offset);
            return *address;
        }

        /**
         * @brief Read a pointer from foreign memory and return wrapped in specified ReClass type
         * @tparam WrapperType The ReClass wrapper type to construct
         * @param offset Offset from base pointer
         * @return Instance of WrapperType wrapping the read pointer, or WrapperType(nullptr) if read fails
         */
        template<typename WrapperType>
        [[nodiscard]] WrapperType ReadPointer(uintptr_t offset) const {
            if (!data()) {
                return WrapperType(nullptr);
            }
            
            void* ptr = nullptr;
            if (!kx::Debug::SafeRead<void*>(data(), offset, ptr)) {
                return WrapperType(nullptr);
            }
            
            return WrapperType(ptr);
        }

        /**
         * @brief FAST pointer read without validation. Use only after validating base pointer and inside guarded regions.
         */
        template<typename WrapperType>
        [[nodiscard]] WrapperType ReadPointerFast(uintptr_t offset) const {
            if (!m_ptr) {
                return WrapperType(nullptr);
            }

            void* ptr = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(m_ptr) + offset);
            return WrapperType(ptr);
        }

        /**
         * @brief Read a typed pointer from foreign memory  
         * @tparam PtrType The pointer type to read (e.g., ChCliCharacter*)
         * @tparam WrapperType The ReClass wrapper type to construct
         * @param offset Offset from base pointer
         * @return Instance of WrapperType wrapping the read pointer, or WrapperType(nullptr) if read fails
         */
        template<typename PtrType, typename WrapperType>
        [[nodiscard]] WrapperType ReadTypedPointer(uintptr_t offset) const {
            if (!data()) {
                return WrapperType(nullptr);
            }
            
            PtrType ptr = nullptr;
            if (!Debug::SafeRead<PtrType>(data(), offset, ptr)) {
                return WrapperType(nullptr);
            }
            
            return WrapperType(ptr);
        }

        /**
         * @brief Read an array pointer from foreign memory
         * @tparam ArrayType The array element type (e.g., ChCliCharacter*)
         * @param offset Offset from base pointer
         * @return Pointer to array, or nullptr if read fails
         */
        template<typename ArrayType>
        [[nodiscard]] ArrayType* ReadArrayPointer(uintptr_t offset) const {
            if (!data()) {
                return nullptr;
            }
            
            ArrayType* arrayPtr = nullptr;
            if (!Debug::SafeRead<ArrayType*>(data(), offset, arrayPtr)) {
                return nullptr;
            }
            
            return arrayPtr;
        }

        /**
         * @brief Safely set a value in foreign memory with offset
         * @tparam T Type to write
         * @param offset Offset from base pointer
         * @param value Value to write
         * @return true if write was successful, false if memory was unsafe
         */
        template <typename T>
        bool set(uintptr_t offset, const T& value) {
            if (!isValidAccess(offset, sizeof(T))) {
                return false; // Cannot set on invalid memory
            }
            
            try {
                *(T*)((uintptr_t)m_ptr + offset) = value;
                return true;
            }
            catch (...) {
                return false;
            }
        }

        /**
         * @brief Safely set a value at base pointer with memory protection handling
         * @tparam T Type to write
         * @param value Value to write
         * @return true if write was successful, false if memory was unsafe
         */
        template <typename T>
        bool setNoOffset(const T& value) {
            if (!isValidAccess(0, sizeof(T))) {
                return false; // Cannot set on invalid memory
            }
            
            uintptr_t uintmPtr = (uintptr_t)m_ptr;
            DWORD oldProtection;
            
            if (!VirtualProtect((LPVOID)uintmPtr, sizeof(T), PAGE_READWRITE, &oldProtection)) {
                return false; // Failed to change protection
            }
            
            bool success = false;
            try {
                *(T*)((uintptr_t)m_ptr) = value;
                success = true;
            }
            catch (...) {
                success = false;
            }
            
            // Always restore the original protection
            VirtualProtect((LPVOID)uintmPtr, sizeof(T), oldProtection, &oldProtection);
            return success;
        }

        /**
         * @brief Safely call a virtual function with memory validation
         * @tparam T Return type
         * @tparam Ts Argument types
         * @param offset The offset from the virtual table base to the function to call
         * @param args The arguments to call the function with
         * @return The return value of the function, or default-constructed T if unsafe
         */
        template <typename T, typename... Ts>
        T call(uintptr_t offset, Ts... args) {
            // Critical validation - this is the most dangerous operation
            if (!isValidForVirtualCall(offset)) {
                return T(); // Return default-constructed value if unsafe
            }
            
            // Additional critical checks for virtual function calls
            if (offset > SafeForeignClassLimits::MAX_REASONABLE_VTABLE_OFFSET) {
                return T(); // Refuse obviously invalid offsets
            }
            
            try {
                uintptr_t vtable_ptr = *(uintptr_t*)m_ptr;
                uintptr_t function_ptr = *(uintptr_t*)(vtable_ptr + offset);
                
                // Critical validation: Check if function pointer looks reasonable
                // Function pointers should be in valid executable memory ranges
                if (function_ptr < SafeForeignClassLimits::MIN_FUNCTION_ADDRESS || 
                    function_ptr > SafeForeignClassLimits::MAX_FUNCTION_ADDRESS) {
                    return T(); // Function pointer outside reasonable range
                }
                
                // Additional validation of function pointer
                if (!SafeAccess::IsMemorySafe((void*)function_ptr, sizeof(void*))) {
                    return T();
                }
                
                // Wrap the virtual function call in exception handling for maximum safety
                T result = T();
                __try {
                    result = ((T(__thiscall*)(void*, Ts...))(function_ptr))(m_ptr, args...);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    // Virtual function call failed - return default value
                    return T();
                }
                return result;
            }
            catch (...) {
                return T(); // Return default value on any exception
            }
        }

        /**
         * @brief Check if this foreign class points to valid memory
         * @return true if the base pointer is valid and safe to access
         */
        [[nodiscard]] bool isValid() const {
            return SafeAccess::IsMemorySafe(m_ptr);
        }

        /**
         * @brief Check if a specific offset is safe to access
         * @param offset Offset from base pointer
         * @param size Size of data to access
         * @return true if the memory range is safe to access
         */
        [[nodiscard]] bool isValidAccess(uintptr_t offset, size_t size) const {
            if (!m_ptr) return false;
            
            // Critical validation: Check for reasonable offset ranges
            if (offset > SafeForeignClassLimits::MAX_REASONABLE_OFFSET) {
                return false; // Refuse obviously invalid offsets
            }
            
            // Critical validation: Check for reasonable access sizes
            if (size > SafeForeignClassLimits::MAX_REASONABLE_SIZE) {
                return false; // Refuse unreasonably large access
            }
            
            // Check for potential overflow in pointer arithmetic
            uintptr_t base_addr = reinterpret_cast<uintptr_t>(m_ptr);
            if (base_addr > UINTPTR_MAX - offset) {
                return false; // Would overflow
            }
            
            void* target_ptr = (void*)(base_addr + offset);
            return SafeAccess::IsMemorySafe(target_ptr, size);
        }

        /**
         * @brief Check if virtual function call is safe
         * @param offset VTable offset
         * @return true if virtual call should be safe
         */
        [[nodiscard]] bool isValidForVirtualCall(uintptr_t offset) const {
            if (!isValidAccess(0, sizeof(uintptr_t))) {
                return false; // Can't even read VTable pointer
            }
            
            // Critical validation: Check for reasonable VTable offset
            if (offset > SafeForeignClassLimits::MAX_REASONABLE_VTABLE_OFFSET) {
                return false; // Refuse obviously invalid VTable offsets
            }
            
            try {
                uintptr_t vtable_ptr = *(uintptr_t*)m_ptr;
                if (vtable_ptr == 0) {
                    return false; // Null VTable
                }
                
                // Critical validation: VTable should be in reasonable memory range
                if (vtable_ptr < SafeForeignClassLimits::MIN_VTABLE_ADDRESS || 
                    vtable_ptr > SafeForeignClassLimits::MAX_VTABLE_ADDRESS) {
                    return false; // VTable outside reasonable range
                }
                
                // Validate VTable pointer itself
                if (!SafeAccess::IsMemorySafe((void*)vtable_ptr, sizeof(uintptr_t))) {
                    return false;
                }
                
                // Validate function pointer location with overflow check
                if (vtable_ptr > UINTPTR_MAX - offset) {
                    return false; // Would overflow
                }
                
                void* function_ptr_location = (void*)(vtable_ptr + offset);
                return SafeAccess::IsMemorySafe(function_ptr_location, sizeof(uintptr_t));
            }
            catch (...) {
                return false;
            }
        }

        explicit operator bool() const { return isValid(); }

        /**
         * @brief Comprehensive validation for critical operations
         * @return true if object passes all critical safety checks
         */
        [[nodiscard]] bool isCriticallyValid() const {
            if (!isValid()) return false;
            
            // Critical check: Ensure the object looks like it has a valid VTable
            if (!isValidAccess(0, sizeof(uintptr_t))) {
                return false;
            }
            
            try {
                uintptr_t potential_vtable = *(uintptr_t*)m_ptr;
                
                // Critical validation: Zero VTable is definitely invalid
                if (potential_vtable == 0) {
                    return false;
                }
                
                // Critical validation: VTable should point to reasonable memory
                if (potential_vtable < SafeForeignClassLimits::MIN_VTABLE_ADDRESS || 
                    potential_vtable > SafeForeignClassLimits::MAX_VTABLE_ADDRESS) {
                    return false;
                }
                
                // Additional check: VTable should be readable
                return SafeAccess::IsMemorySafe((void*)potential_vtable, sizeof(uintptr_t));
            }
            catch (...) {
                return false;
            }
        }

        void* data() { return m_ptr; }
        [[nodiscard]] const void* data() const { return m_ptr; }

        /**
         * @brief Get the raw pointer value as an address (for debugging/logging)
         * @return The pointer as a uintptr_t, or 0 if null
         */
        [[nodiscard]] uintptr_t address() const {
            return reinterpret_cast<uintptr_t>(m_ptr);
        }

        /**
         * @brief Reset the pointer to null (makes object invalid)
         */
        void reset() {
            m_ptr = nullptr;
        }

        /**
         * @brief Reset with a new pointer (validates the new pointer)
         * @param ptr New pointer to wrap
         */
        void reset(void* ptr) {
            m_ptr = ptr;
            // Validate the new pointer
            if (m_ptr && !SafeAccess::IsMemorySafe(m_ptr)) {
                m_ptr = nullptr;
            }
        }

    private:
        void* m_ptr;
    };

    // --- Comparison operators ---
    inline bool operator==(const SafeForeignClass& lhs, const SafeForeignClass& rhs) {
        return lhs.data() == rhs.data();
    }

    inline bool operator==(const SafeForeignClass& lhs, nullptr_t) {
        return lhs.data() == nullptr;
    }

    inline bool operator==(nullptr_t, const SafeForeignClass& rhs) {
        return nullptr == rhs.data();
    }

    inline bool operator!=(const SafeForeignClass& lhs, const SafeForeignClass& rhs) {
        return lhs.data() != rhs.data();
    }

    inline bool operator!=(const SafeForeignClass& lhs, nullptr_t) {
        return lhs.data() != nullptr;
    }

    inline bool operator!=(nullptr_t, const SafeForeignClass& rhs) {
        return nullptr != rhs.data();
    }

    // --- Arithmetic operators with overflow protection ---
    inline SafeForeignClass operator+(const SafeForeignClass& base, uintptr_t offset) {
        uintptr_t base_addr = reinterpret_cast<uintptr_t>(base.data());
        
        // Critical validation: Check for overflow
        if (base_addr > UINTPTR_MAX - offset) {
            return SafeForeignClass(nullptr); // Prevent overflow
        }
        
        // Critical validation: Check for reasonable offset
        if (offset > SafeForeignClassLimits::MAX_REASONABLE_OFFSET) {
            return SafeForeignClass(nullptr); // Prevent unreasonable offsets
        }
        
        return SafeForeignClass((void*)(base_addr + offset));
    }

    inline SafeForeignClass operator+(uintptr_t offset, const SafeForeignClass& base) {
        return base + offset;
    }

    inline SafeForeignClass operator-(const SafeForeignClass& base, uintptr_t offset) {
        uintptr_t base_addr = reinterpret_cast<uintptr_t>(base.data());
        
        // Critical validation: Check for underflow
        if (base_addr < offset) {
            return SafeForeignClass(nullptr); // Prevent underflow
        }
        
        // Critical validation: Check for reasonable offset
        if (offset > SafeForeignClassLimits::MAX_REASONABLE_OFFSET) {
            return SafeForeignClass(nullptr); // Prevent unreasonable offsets
        }
        
        return SafeForeignClass((void*)(base_addr - offset));
    }

} // namespace kx