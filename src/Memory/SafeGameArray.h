#pragma once

#include <iterator>
#include <cstdint>
#include "Safety.h"

namespace kx {
namespace SafeAccess {

    /**
     * @brief Generic safe iterator for game array structures
     * 
     * This template class provides safe iteration over game arrays with automatic
     * validation of memory addresses, VTable pointers, and wrapper validity.
     * 
     * @tparam WrapperType The wrapper class type (e.g., ChCliCharacter, ChCliPlayer, etc.)
     */
    template <typename WrapperType>
    class SafeGameArray {
    public:
        class Iterator {
        private:
            void** m_ptr;
            uint32_t m_idx;
            uint32_t m_cap;
            mutable WrapperType m_current;

            void advance_to_valid() {
                m_current = WrapperType(nullptr);
                while (m_idx < m_cap) {
                    void* candidate = m_ptr[m_idx];
                    
                    if (IsValidGameObject(candidate)) {
                        WrapperType wrapper(candidate);
                        if (wrapper.isValid()) {
                            m_current = wrapper;
                            return;
                        }
                    }
                    m_idx++;
                }
            }

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = WrapperType;
            using difference_type = std::ptrdiff_t;
            using pointer = const WrapperType*;
            using reference = const WrapperType&;

            Iterator(void** ptr, uint32_t idx, uint32_t cap)
                : m_ptr(ptr), m_idx(idx), m_cap(cap), m_current(nullptr) {
                if (m_ptr && m_idx < m_cap) {
                    advance_to_valid();
                }
            }

            Iterator& operator++() {
                if (m_idx < m_cap) {
                    m_idx++;
                    advance_to_valid();
                }
                return *this;
            }

            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const Iterator& other) const {
                return m_ptr == other.m_ptr && m_idx == other.m_idx;
            }

            bool operator!=(const Iterator& other) const {
                return !(*this == other);
            }

            const WrapperType& operator*() const {
                return m_current;
            }

            const WrapperType* operator->() const {
                return &m_current;
            }
        };

    private:
        void** m_rawArray;
        uint32_t m_capacity;

    public:
        SafeGameArray(void* ptrArray, uint32_t capacity)
            : m_rawArray(reinterpret_cast<void**>(ptrArray)), m_capacity(capacity) {
            if (!m_rawArray) {
                m_capacity = 0;
            }
        }

        Iterator begin() const {
            return Iterator(m_rawArray, 0, m_capacity);
        }

        Iterator end() const {
            return Iterator(m_rawArray, m_capacity, m_capacity);
        }
    };

} // namespace SafeAccess
} // namespace kx
