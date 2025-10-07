#pragma once
#include <cmath>

namespace kx {
namespace Animation {

    // An "Ease Out" function. Starts fast, ends slow.
    // Takes progress from 0.0 to 1.0 and returns the eased progress from 0.0 to 1.0.
    inline float EaseOutCubic(float progress) {
        return 1.0f - pow(1.0f - progress, 3.0f);
    }

    // A more dramatic "Ease Out" function.
    inline float EaseOutQuint(float progress) {
        return 1.0f - pow(1.0f - progress, 5.0f);
    }

} // namespace Animation
} // namespace kx
