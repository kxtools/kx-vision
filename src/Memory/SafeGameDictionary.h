#pragma once

#include <iterator>
#include <cstdint>
#include <optional>
#include <limits>
#include "Safety.h"

namespace kx {
namespace SafeAccess {

    /**
     * @brief Key-Value pair for game dictionary structures
     * 
     * Represents a dictionary entry with a key (typically uint32_t) and a value (wrapper object).
     * The layout matches GW2's dictionary structure:
     * - Key (4 bytes)
     * - Padding (4 bytes)
     * - Pointer to value object (8 bytes)
     * 
     * @tparam KeyType The type of the key (default: uint32_t)
     * @tparam ValueType The wrapper class type for the value (e.g., PoiCliUnlock)
     */
    template <typename KeyType, typename ValueType>
    struct DictionaryEntry {
        KeyType key;
        ValueType value;

        DictionaryEntry(KeyType k, ValueType v) : key(k), value(v) {}
    };

    /**
     * @brief Generic safe iterator for game dictionary structures
     * 
     * This template class provides safe iteration over game dictionaries with automatic
     * validation of memory addresses, VTable pointers, and wrapper validity.
     * Exposes key-value pairs in a dictionary-like manner.
     * 
     * Dictionary structure layout (GW2):
     * Offset 0x00: Key (4 bytes, uint32_t)
     * Offset 0x04: Padding (4 bytes)
     * Offset 0x08: Pointer to value object (8 bytes)
     * Total stride: 0x18 bytes (includes additional internal data)
     * 
     * @tparam KeyType The type of the key (default: uint32_t)
     * @tparam ValueType The wrapper class type (e.g., PoiCliUnlock)
     */
    template <typename KeyType, typename ValueType>
    class SafeGameDictionary {
    public:
        using Entry = DictionaryEntry<KeyType, ValueType>;

        class Iterator {
        private:
            uint8_t* m_base;      // Base pointer (byte-addressable)
            uint32_t m_idx;
            uint32_t m_cap;
            size_t m_stride;      // Bytes between elements
            size_t m_keyOffset;   // Offset to key (typically 0)
            size_t m_ptrOffset;   // Offset to pointer within each element
            mutable Entry m_current;

            void advance_to_valid() {
                m_current = Entry(KeyType{}, ValueType(nullptr));
                while (m_idx < m_cap) {
                    const uintptr_t entryAddress = reinterpret_cast<uintptr_t>(m_base) +
                                                   static_cast<uintptr_t>(m_idx) * m_stride;

                    KeyType key{};
                    if (!TryRead(entryAddress + m_keyOffset, key)) {
                        // Unreadable backing storage invalidates layout assumptions; stop instead of skipping ahead.
                        m_idx = m_cap;
                        return;
                    }

                    void* candidate = nullptr;
                    if (!TryRead(entryAddress + m_ptrOffset, candidate)) {
                        m_idx = m_cap;
                        return;
                    }
                    
                    // Dictionary values may be plain structures without VTables.
                    ValueType wrapper(candidate);
                    if (wrapper.isValid()) {
                        m_current = Entry(key, wrapper);
                        return;
                    }
                    m_idx++;
                }
            }

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = Entry;
            using difference_type = std::ptrdiff_t;
            using pointer = const Entry*;
            using reference = const Entry&;

            Iterator(uint8_t* base, uint32_t idx, uint32_t cap, size_t stride, size_t keyOffset, size_t ptrOffset)
                : m_base(base), m_idx(idx), m_cap(cap), m_stride(stride), 
                  m_keyOffset(keyOffset), m_ptrOffset(ptrOffset), 
                  m_current(KeyType{}, ValueType(nullptr)) {
                if (m_base && m_idx < m_cap) {
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
                return m_base == other.m_base && m_idx == other.m_idx;
            }

            bool operator!=(const Iterator& other) const {
                return !(*this == other);
            }

            const Entry& operator*() const {
                return m_current;
            }

            const Entry* operator->() const {
                return &m_current;
            }
        };

    private:
        uint8_t* m_rawArray;
        uint32_t m_capacity;
        size_t m_stride;
        size_t m_keyOffset;
        size_t m_ptrOffset;

    public:
        /**
         * @brief Constructor for dictionaries with custom stride
         * @param ptrArray Pointer to the dictionary array
         * @param capacity Number of elements in the dictionary
         * @param stride Bytes between elements (e.g., 0x18 for GW2 dictionaries)
         * @param ptrOffset Offset to pointer within each element (e.g., 0x08)
         * @param keyOffset Offset to key within each element (default: 0x00)
         */
        SafeGameDictionary(void* ptrArray, uint32_t capacity, size_t stride, size_t ptrOffset, size_t keyOffset = 0)
            : m_rawArray(reinterpret_cast<uint8_t*>(ptrArray)), 
              m_capacity(capacity),
              m_stride(stride),
              m_keyOffset(keyOffset),
              m_ptrOffset(ptrOffset) {
            const uintptr_t base = reinterpret_cast<uintptr_t>(m_rawArray);
            // Stride and offsets are foreign-layout inputs. Validate once so iterator arithmetic stays bounded.
            const bool invalidFields = m_stride == 0 || m_keyOffset > m_stride ||
                sizeof(KeyType) > m_stride - m_keyOffset || m_ptrOffset > m_stride ||
                sizeof(void*) > m_stride - m_ptrOffset;
            const bool invalidCapacity = m_capacity > MAX_GAME_CONTAINER_CAPACITY ||
                (m_stride != 0 && m_capacity > (std::numeric_limits<size_t>::max)() / m_stride);
            const size_t byteSize = invalidCapacity ? 0 : static_cast<size_t>(m_capacity) * m_stride;

            if (!m_rawArray || invalidFields || invalidCapacity ||
                (m_capacity > 0 && !IsRangeInBounds(base, byteSize))) {
                m_rawArray = nullptr;
                m_capacity = 0;
            }
        }

        Iterator begin() const {
            return Iterator(m_rawArray, 0, m_capacity, m_stride, m_keyOffset, m_ptrOffset);
        }

        Iterator end() const {
            return Iterator(m_rawArray, m_capacity, m_capacity, m_stride, m_keyOffset, m_ptrOffset);
        }

        /**
         * @brief Find an entry by key
         * @param key The key to search for
         * @return Optional containing the value wrapper if found
         */
        std::optional<ValueType> find(KeyType key) const {
            if (!m_rawArray) {
                return std::nullopt;
            }

            for (uint32_t i = 0; i < m_capacity; ++i) {
                const uintptr_t entryAddress = reinterpret_cast<uintptr_t>(m_rawArray) +
                                               static_cast<uintptr_t>(i) * m_stride;

                KeyType entryKey{};
                if (!TryRead(entryAddress + m_keyOffset, entryKey)) {
                    return std::nullopt;
                }

                if (entryKey == key) {
                    void* candidate = nullptr;
                    if (!TryRead(entryAddress + m_ptrOffset, candidate)) {
                        return std::nullopt;
                    }
                    
                    ValueType wrapper(candidate);
                    if (wrapper.isValid()) {
                        return wrapper;
                    }
                }
            }
            
            return std::nullopt;
        }

        /**
         * @brief Check if a key exists in the dictionary
         * @param key The key to check
         * @return true if the key exists and has a valid value
         */
        bool contains(KeyType key) const {
            return find(key).has_value();
        }

        /**
         * @brief Get the number of valid entries in the dictionary
         * @return Count of valid entries
         */
        uint32_t size() const {
            uint32_t count = 0;
            for (const auto& entry : *this) {
                (void)entry; // Suppress unused warning
                count++;
            }
            return count;
        }

        /**
         * @brief Check if the dictionary is empty
         * @return true if no valid entries exist
         */
        bool empty() const {
            return begin() == end();
        }
    };

} // namespace SafeAccess
} // namespace kx
