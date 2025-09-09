#pragma once

#include <windows.h> // Required for VirtualProtect

namespace kx
{
    /* C++ style casting is avoided for readability. */

    // A foreign class whose layout is defined at runtime
    class ForeignClass
    {
    public:
        ForeignClass(void* ptr) : m_ptr(ptr) {}

        template <typename T>
        [[nodiscard]] T get(uintptr_t offset) const
        {
            if (m_ptr == nullptr) {
                // Return a default-constructed T or throw an exception
                // For primitive types, 0 or false is a safe default.
                // For complex types, this might be problematic.
                // For now, return a default-constructed value.
                return T();
            }
            return *(T*)((uintptr_t)m_ptr + offset);
        }

        template <typename T>
        void set(uintptr_t offset, T const& value)
        {
            if (m_ptr == nullptr) {
                return; // Cannot set on a null pointer
            }
            *(T*)((uintptr_t)m_ptr + offset) = value;
        }

        template <typename T> void setNoOffset(T const& value)
        {
            if (m_ptr == nullptr) {
                return; // Cannot set on a null pointer
            }
            uintptr_t uintmPtr = (uintptr_t)m_ptr;
            DWORD oldProtection;
            if (VirtualProtect((LPVOID)uintmPtr, sizeof(T), PAGE_READWRITE, &oldProtection))
            {
                *(T*)((uintptr_t)m_ptr) = value;
                // Restore the original protection
                VirtualProtect((LPVOID)uintmPtr, sizeof(T), oldProtection, &oldProtection);
            }
        }

        /**
         * Calls a virtual function of the foreign class instance.
         *
         * \param offset The offset from the virtual table base to the function to call
         * \param args The arguments to call the function with
         * \return The return value of the function that was called
         */
        template <typename T, typename... Ts>
        T call(uintptr_t offset, Ts... args)
        {
            if (m_ptr == nullptr) {
                // Cannot call virtual function on a null pointer
                return T(); // Return default-constructed value for return type
            }
            // Add a check for the VTable pointer itself
            uintptr_t vtable_ptr = *(uintptr_t*)m_ptr;
            if (vtable_ptr == 0) {
                return T(); // Return default-constructed value if VTable is null
            }

            return ((T(__thiscall*)(void*, Ts...))(*(uintptr_t*)(vtable_ptr + offset)))(m_ptr, args...);
        }

        explicit operator bool() const { return m_ptr != nullptr; }

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

