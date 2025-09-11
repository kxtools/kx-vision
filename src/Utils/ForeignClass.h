#pragma once

#include <windows.h> // Required for VirtualProtect
#include "MemorySafety.h" // Include our memory safety utilities

namespace kx
{
    /* C++ style casting is avoided for readability. */

    // A foreign class whose layout is defined at runtime
    // Note: Consider migrating to SafeForeignClass for better memory safety
    class ForeignClass
    {
    public:
        ForeignClass(void* ptr) : m_ptr(ptr) {}

        template <typename T>
        [[nodiscard]] T get(uintptr_t offset) const
        {
            if (m_ptr == nullptr) {
                return T(); // Return default-constructed value for null pointer
            }
            
            // Add basic memory safety check
            void* target_ptr = (void*)((uintptr_t)m_ptr + offset);
            if (!SafeAccess::IsMemorySafe(target_ptr, sizeof(T))) {
                return T(); // Return default value for unsafe memory
            }
            
            try {
                return *(T*)((uintptr_t)m_ptr + offset);
            }
            catch (...) {
                return T(); // Return default value on exception
            }
        }

        template <typename T>
        bool set(uintptr_t offset, T const& value)
        {
            if (m_ptr == nullptr) {
                return false; // Cannot set on a null pointer
            }
            
            // Add basic memory safety check
            void* target_ptr = (void*)((uintptr_t)m_ptr + offset);
            if (!SafeAccess::IsMemorySafe(target_ptr, sizeof(T))) {
                return false; // Cannot set on unsafe memory
            }
            
            try {
                *(T*)((uintptr_t)m_ptr + offset) = value;
                return true;
            }
            catch (...) {
                return false; // Failed to set value
            }
        }

        template <typename T> 
        bool setNoOffset(T const& value)
        {
            if (m_ptr == nullptr) {
                return false; // Cannot set on a null pointer
            }
            
            // Add basic memory safety check
            if (!SafeAccess::IsMemorySafe(m_ptr, sizeof(T))) {
                return false; // Cannot set on unsafe memory
            }
            
            uintptr_t uintmPtr = (uintptr_t)m_ptr;
            DWORD oldProtection;
            if (!VirtualProtect((LPVOID)uintmPtr, sizeof(T), PAGE_READWRITE, &oldProtection))
            {
                return false; // Failed to change protection
            }
            
            bool success = false;
            try {
                *(T*)((uintptr_t)m_ptr) = value;
                success = true;
            }
            catch (...) {
                success = false; // Exception occurred
            }
            
            // Always restore the original protection
            VirtualProtect((LPVOID)uintmPtr, sizeof(T), oldProtection, &oldProtection);
            return success;
        }

        /**
         * Calls a virtual function of the foreign class instance with safety checks.
         *
         * \param offset The offset from the virtual table base to the function to call
         * \param args The arguments to call the function with
         * \return The return value of the function that was called, or default T if unsafe
         */
        template <typename T, typename... Ts>
        T call(uintptr_t offset, Ts... args)
        {
            if (m_ptr == nullptr) {
                return T(); // Cannot call virtual function on a null pointer
            }
            
            // Add memory safety checks for VTable access
            if (!SafeAccess::IsMemorySafe(m_ptr, sizeof(uintptr_t))) {
                return T(); // Cannot safely read VTable pointer
            }
            
            try {
                uintptr_t vtable_ptr = *(uintptr_t*)m_ptr;
                if (vtable_ptr == 0) {
                    return T(); // Return default-constructed value if VTable is null
                }
                
                // Validate VTable pointer
                if (!SafeAccess::IsMemorySafe((void*)vtable_ptr, sizeof(uintptr_t))) {
                    return T(); // VTable pointer is invalid
                }
                
                // Validate function pointer location
                void* function_ptr_location = (void*)(vtable_ptr + offset);
                if (!SafeAccess::IsMemorySafe(function_ptr_location, sizeof(uintptr_t))) {
                    return T(); // Function pointer location is invalid
                }
                
                uintptr_t function_ptr = *(uintptr_t*)(vtable_ptr + offset);
                if (!SafeAccess::IsMemorySafe((void*)function_ptr, sizeof(void*))) {
                    return T(); // Function pointer itself is invalid
                }
                
                return ((T(__thiscall*)(void*, Ts...))(function_ptr))(m_ptr, args...);
            }
            catch (...) {
                return T(); // Return default value on any exception
            }
        }

        explicit operator bool() const { return m_ptr != nullptr && SafeAccess::IsMemorySafe(m_ptr); }

        /**
         * @brief Check if this foreign class points to valid, safe memory
         * @return true if the memory is safe to access
         */
        [[nodiscard]] bool isValid() const { 
            return SafeAccess::IsMemorySafe(m_ptr); 
        }

        void* data() { return m_ptr; }

        [[nodiscard]] const void* data() const { return m_ptr; }

    private:
        void* m_ptr;
    };


    inline bool operator==(const ForeignClass& lhs, const ForeignClass& rhs)
    {
        return lhs.data() == rhs.data();
    }

    inline bool operator==(const ForeignClass& lhs, nullptr_t)
    {
        return lhs.data() == nullptr;
    }

    inline bool operator==(nullptr_t, const ForeignClass& rhs)
    {
        return nullptr == rhs.data();
    }

    inline bool operator!=(const ForeignClass& lhs, const ForeignClass& rhs)
    {
        return lhs.data() != rhs.data();
    }

    inline bool operator!=(const ForeignClass& lhs, nullptr_t)
    {
        return lhs.data() != nullptr;
    }

    inline bool operator!=(nullptr_t, const ForeignClass& rhs)
    {
        return nullptr != rhs.data();
    }


    inline ForeignClass operator+(const ForeignClass& base, uintptr_t offset)
    {
        return (void*)((uintptr_t)base.data() + offset);
    }

    inline ForeignClass operator+(uintptr_t offset, const ForeignClass& base)
    {
        return base + offset;
    }

    inline ForeignClass operator-(const ForeignClass& base, uintptr_t offset)
    {
        return (void*)((uintptr_t)base.data() - offset);
    }

    inline ForeignClass operator-(uintptr_t offset, const ForeignClass& base)
    {
        return base - offset;
    }
}

