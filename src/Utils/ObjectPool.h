#pragma once
#include <vector>
#include <memory>

namespace kx {

/**
 * @brief Simple object pool to eliminate heap allocations in main loops
 * 
 * Pre-allocates a fixed number of objects and allows checking them out/returning them
 * to avoid repeated new/delete operations that cause heap churn and performance drops.
 */
template<typename T>
class ObjectPool {
private:
    std::vector<T> m_pool;
    size_t m_nextAvailable = 0;

public:
    /**
     * @brief Constructor - pre-allocates the specified number of objects
     * @param initialSize Number of objects to pre-allocate in the pool
     */
    explicit ObjectPool(size_t initialSize) {
        m_pool.resize(initialSize);
    }

    /**
     * @brief Get an object from the pool
     * @return Pointer to an available object, or nullptr if pool is exhausted
     */
    T* Get() {
        if (m_nextAvailable >= m_pool.size()) {
            // Pool is exhausted. In a real-world scenario you might resize,
            // but for ESP, we can just return nullptr and skip rendering extras.
            return nullptr; 
        }
        return &m_pool[m_nextAvailable++];
    }

    /**
     * @brief Reset the pool to reuse all objects
     * Call this at the beginning of each frame to "return" all objects to the pool
     */
    void Reset() {
        m_nextAvailable = 0;
    }

    /**
     * @brief Get the total size of the pool
     */
    size_t Size() const {
        return m_pool.size();
    }

    /**
     * @brief Get the number of objects currently checked out
     */
    size_t Used() const {
        return m_nextAvailable;
    }

    /**
     * @brief Get the number of objects still available
     */
    size_t Available() const {
        return m_pool.size() - m_nextAvailable;
    }
};

} // namespace kx