#pragma once

#include <iterator>
#include "../Game/ReClassStructs.h"
#include "MemorySafety.h"

namespace kx {
namespace SafeAccess {

    /**
     * @brief Safe iterator for character arrays from the game
     * 
     * This iterator wraps raw ChCliCharacter** arrays and provides safe iteration
     * with automatic validation of memory addresses and null pointer checks.
     */
    class CharacterListIterator {
    private:
        ReClass::ChCliCharacter** m_array;
        uint32_t m_index;
        uint32_t m_capacity;
        mutable ReClass::ChCliCharacter m_current;
        mutable bool m_currentValid;

        void AdvanceToValid() {
            m_currentValid = false;
            while (m_index < m_capacity) {
                if (IsMemorySafe(m_array[m_index]) && IsVTablePointerValid(m_array[m_index])) {
                    m_current = ReClass::ChCliCharacter(m_array[m_index]);
                    if (m_current) {
                        m_currentValid = true;
                        return;
                    }
                }
                ++m_index;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ReClass::ChCliCharacter;
        using difference_type = std::ptrdiff_t;
        using pointer = const ReClass::ChCliCharacter*;
        using reference = const ReClass::ChCliCharacter&;

        CharacterListIterator(ReClass::ChCliCharacter** array, uint32_t index, uint32_t capacity)
            : m_array(array), m_index(index), m_capacity(capacity), m_current(nullptr), m_currentValid(false) {
            if (m_array && m_capacity < MAX_REASONABLE_CHARACTER_COUNT) { // Sanity check
                AdvanceToValid();
            } else {
                m_index = m_capacity; // Mark as end
            }
        }

        CharacterListIterator& operator++() {
            if (m_index < m_capacity) {
                ++m_index;
                AdvanceToValid();
            }
            return *this;
        }

        CharacterListIterator operator++(int) {
            CharacterListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        const ReClass::ChCliCharacter& operator*() const {
            return m_current;
        }

        const ReClass::ChCliCharacter* operator->() const {
            return &m_current;
        }

        bool operator==(const CharacterListIterator& other) const {
            return m_array == other.m_array && m_index == other.m_index;
        }

        bool operator!=(const CharacterListIterator& other) const {
            return !(*this == other);
        }

        bool IsValid() const {
            return m_currentValid;
        }
    };

    /**
     * @brief Safe iterator for player arrays from the game
     * 
     * This iterator wraps raw ChCliPlayer** arrays and provides safe iteration
     * with automatic validation and character extraction.
     */
    class PlayerListIterator {
    private:
        ReClass::ChCliPlayer** m_array;
        uint32_t m_index;
        uint32_t m_capacity;
        mutable ReClass::ChCliPlayer m_currentPlayer;
        mutable ReClass::ChCliCharacter m_currentCharacter;
        mutable const wchar_t* m_currentName;
        mutable bool m_currentValid;

        void AdvanceToValid() {
            m_currentValid = false;
            m_currentName = nullptr;
            while (m_index < m_capacity) {
                if (IsMemorySafe(m_array[m_index]) && IsVTablePointerValid(m_array[m_index])) {
                    m_currentPlayer = ReClass::ChCliPlayer(m_array[m_index]);
                    if (m_currentPlayer) {
                        m_currentCharacter = m_currentPlayer.GetCharacter();
                        m_currentName = m_currentPlayer.GetName();
                        if (m_currentCharacter.data() && m_currentName) {
                            m_currentValid = true;
                            return;
                        }
                    }
                }
                ++m_index;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<ReClass::ChCliCharacter, const wchar_t*>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        PlayerListIterator(ReClass::ChCliPlayer** array, uint32_t index, uint32_t capacity)
            : m_array(array), m_index(index), m_capacity(capacity), 
              m_currentPlayer(nullptr), m_currentCharacter(nullptr), 
              m_currentName(nullptr), m_currentValid(false) {
            if (m_array && m_capacity < MAX_REASONABLE_PLAYER_COUNT) { // Sanity check for player count
                AdvanceToValid();
            } else {
                m_index = m_capacity; // Mark as end
            }
        }

        PlayerListIterator& operator++() {
            if (m_index < m_capacity) {
                ++m_index;
                AdvanceToValid();
            }
            return *this;
        }

        PlayerListIterator operator++(int) {
            PlayerListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // Return the character (for rendering)
        const ReClass::ChCliCharacter& GetCharacter() const {
            return m_currentCharacter;
        }

        // Return the player name
        const wchar_t* GetName() const {
            return m_currentName;
        }

        // Return player character data pointer (for mapping)
        void* GetCharacterDataPtr() const {
            return m_currentCharacter.data();
        }

        bool operator==(const PlayerListIterator& other) const {
            return m_array == other.m_array && m_index == other.m_index;
        }

        bool operator!=(const PlayerListIterator& other) const {
            return !(*this == other);
        }

        bool IsValid() const {
            return m_currentValid;
        }
    };

    /**
     * @brief Safe iterator for gadget arrays from the game
     * 
     * This iterator wraps raw GdCliGadget** arrays and provides safe iteration
     * with automatic validation.
     */
    class GadgetListIterator {
    private:
        ReClass::GdCliGadget** m_array;
        uint32_t m_index;
        uint32_t m_capacity;
        mutable ReClass::GdCliGadget m_current;
        mutable bool m_currentValid;

        void AdvanceToValid() {
            m_currentValid = false;
            while (m_index < m_capacity) {
                if (IsMemorySafe(m_array[m_index]) && IsVTablePointerValid(m_array[m_index])) {
                    m_current = ReClass::GdCliGadget(m_array[m_index]);
                    if (m_current) {
                        m_currentValid = true;
                        return;
                    }
                }
                ++m_index;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ReClass::GdCliGadget;
        using difference_type = std::ptrdiff_t;
        using pointer = const ReClass::GdCliGadget*;
        using reference = const ReClass::GdCliGadget&;

        GadgetListIterator(ReClass::GdCliGadget** array, uint32_t index, uint32_t capacity)
            : m_array(array), m_index(index), m_capacity(capacity), m_current(nullptr), m_currentValid(false) {
            if (m_array && m_capacity < MAX_REASONABLE_GADGET_COUNT) { // Sanity check
                AdvanceToValid();
            } else {
                m_index = m_capacity; // Mark as end
            }
        }

        GadgetListIterator& operator++() {
            if (m_index < m_capacity) {
                ++m_index;
                AdvanceToValid();
            }
            return *this;
        }

        GadgetListIterator operator++(int) {
            GadgetListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        const ReClass::GdCliGadget& operator*() const {
            return m_current;
        }

        const ReClass::GdCliGadget* operator->() const {
            return &m_current;
        }

        bool operator==(const GadgetListIterator& other) const {
            return m_array == other.m_array && m_index == other.m_index;
        }

        bool operator!=(const GadgetListIterator& other) const {
            return !(*this == other);
        }

        bool IsValid() const {
            return m_currentValid;
        }
    };

    /**
     * @brief Safe iterator for attack target list arrays from the game
     * 
     * This iterator wraps raw AttackTargetListEntry** arrays and provides safe iteration
     * with automatic validation and AgKeyframed extraction.
     */
    class AttackTargetListIterator {
    private:
        ReClass::AttackTargetListEntry** m_array;
        uint32_t m_index;
        uint32_t m_capacity;
        mutable ReClass::AttackTargetListEntry m_currentEntry;
        mutable ReClass::AgKeyFramed m_currentAgKeyframed;
        mutable bool m_currentValid;

        void AdvanceToValid() {
            m_currentValid = false;
            while (m_index < m_capacity) {
                if (IsMemorySafe(m_array[m_index])) {
                    m_currentEntry = ReClass::AttackTargetListEntry(m_array[m_index]);
                    if (m_currentEntry) {
                        m_currentAgKeyframed = m_currentEntry.GetAgKeyFramed();
                        if (m_currentAgKeyframed && IsVTablePointerValid(m_currentAgKeyframed.data())) {
                            m_currentValid = true;
                            return;
                        }
                    }
                }
                ++m_index;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ReClass::AgKeyFramed;
        using difference_type = std::ptrdiff_t;
        using pointer = const ReClass::AgKeyFramed*;
        using reference = const ReClass::AgKeyFramed&;

        AttackTargetListIterator(ReClass::AttackTargetListEntry** array, uint32_t index, uint32_t capacity)
            : m_array(array), m_index(index), m_capacity(capacity), 
              m_currentEntry(nullptr), m_currentAgKeyframed(nullptr), m_currentValid(false) {
            if (m_array && m_capacity < MAX_REASONABLE_ATTACK_TARGET_COUNT) {
                AdvanceToValid();
            } else {
                m_index = m_capacity;
            }
        }

        AttackTargetListIterator& operator++() {
            if (m_index < m_capacity) {
                ++m_index;
                AdvanceToValid();
            }
            return *this;
        }

        AttackTargetListIterator operator++(int) {
            AttackTargetListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        const ReClass::AgKeyFramed& operator*() const {
            return m_currentAgKeyframed;
        }

        const ReClass::AgKeyFramed* operator->() const {
            return &m_currentAgKeyframed;
        }

        bool operator==(const AttackTargetListIterator& other) const {
            return m_array == other.m_array && m_index == other.m_index;
        }

        bool operator!=(const AttackTargetListIterator& other) const {
            return !(*this == other);
        }

        bool IsValid() const {
            return m_currentValid;
        }
    };

    /**
     * @brief Range wrapper for character lists to enable range-based for loops
     * 
     * Iterates over the full CAPACITY to ensure no valid entries are missed.
     * The iterator automatically skips null and invalid entries.
     */
    class CharacterList {
    private:
        ReClass::ChCliCharacter** m_array;
        uint32_t m_capacity;

    public:
        CharacterList(ReClass::ChCliContext& context) {
            if (context) {
                m_array = context.GetCharacterList();
                m_capacity = context.GetCharacterListCapacity();
            } else {
                m_array = nullptr;
                m_capacity = 0;
            }
        }

        CharacterListIterator begin() const {
            return CharacterListIterator(m_array, 0, m_capacity);
        }

        CharacterListIterator end() const {
            return CharacterListIterator(m_array, m_capacity, m_capacity);
        }
    };

    /**
     * @brief Range wrapper for player lists to enable range-based for loops
     * 
     * Iterates over the full CAPACITY to ensure no valid entries are missed.
     * The iterator automatically skips null and invalid entries.
     */
    class PlayerList {
    private:
        ReClass::ChCliPlayer** m_array;
        uint32_t m_capacity;

    public:
        PlayerList(ReClass::ChCliContext& context) {
            if (context) {
                m_array = context.GetPlayerList();
                m_capacity = context.GetPlayerListCapacity();
            } else {
                m_array = nullptr;
                m_capacity = 0;
            }
        }

        PlayerListIterator begin() const {
            return PlayerListIterator(m_array, 0, m_capacity);
        }

        PlayerListIterator end() const {
            return PlayerListIterator(m_array, m_capacity, m_capacity);
        }
    };

    /**
     * @brief Range wrapper for gadget lists to enable range-based for loops
     * 
     * Iterates over the full CAPACITY to ensure no valid entries are missed.
     * The iterator automatically skips null and invalid entries.
     */
    class GadgetList {
    private:
        ReClass::GdCliGadget** m_array;
        uint32_t m_capacity;

    public:
        GadgetList(ReClass::GdCliContext& context) {
            if (context) {
                m_array = context.GetGadgetList();
                m_capacity = context.GetGadgetListCapacity();
            } else {
                m_array = nullptr;
                m_capacity = 0;
            }
        }

        GadgetListIterator begin() const {
            return GadgetListIterator(m_array, 0, m_capacity);
        }

        GadgetListIterator end() const {
            return GadgetListIterator(m_array, m_capacity, m_capacity);
        }
    };

    /**
     * @brief Range wrapper for attack target lists to enable range-based for loops
     * 
     * Iterates over the full CAPACITY to ensure no valid entries are missed.
     * The iterator automatically skips null and invalid entries.
     */
    class AttackTargetList {
    private:
        ReClass::AttackTargetListEntry** m_array;
        uint32_t m_capacity;

    public:
        AttackTargetList(ReClass::GdCliContext& context) {
            if (context) {
                m_array = context.GetAttackTargetList();
                m_capacity = context.GetAttackTargetListCapacity();
            } else {
                m_array = nullptr;
                m_capacity = 0;
            }
        }

        AttackTargetListIterator begin() const {
            return AttackTargetListIterator(m_array, 0, m_capacity);
        }

        AttackTargetListIterator end() const {
            return AttackTargetListIterator(m_array, m_capacity, m_capacity);
        }
    };

} // namespace SafeAccess
} // namespace kx