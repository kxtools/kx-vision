#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/vec3.hpp>

namespace kx {

struct TrailPoint {
    glm::vec3 position{0.0f};
    uint64_t timestamp = 0;
};

constexpr size_t MAX_TRAIL_HISTORY_CAPACITY = 64;

struct TrailHistory {
    std::array<TrailPoint, MAX_TRAIL_HISTORY_CAPACITY> points{};
    size_t head = 0;
    size_t size = 0;

    void Limit(size_t maxPoints) noexcept {
        if (head >= MAX_TRAIL_HISTORY_CAPACITY) {
            head = 0;
        }
        if (size > MAX_TRAIL_HISTORY_CAPACITY) {
            size = MAX_TRAIL_HISTORY_CAPACITY;
        }
        if (size > maxPoints) {
            size = maxPoints;
        }
    }

    void Push(const TrailPoint& point, size_t maxPoints) noexcept {
        Limit(maxPoints);
        if (maxPoints == 0) {
            return;
        }
        points[head] = point;
        head = (head + 1) % MAX_TRAIL_HISTORY_CAPACITY;
        if (size < maxPoints) {
            ++size;
        }
    }

    [[nodiscard]] const TrailPoint& Get(size_t index) const noexcept {
        if (head >= MAX_TRAIL_HISTORY_CAPACITY ||
            size > MAX_TRAIL_HISTORY_CAPACITY || index >= size) {
            return points[0];
        }
        const size_t tail = (head + MAX_TRAIL_HISTORY_CAPACITY - size) % MAX_TRAIL_HISTORY_CAPACITY;
        return points[(tail + index) % MAX_TRAIL_HISTORY_CAPACITY];
    }
};

} // namespace kx
