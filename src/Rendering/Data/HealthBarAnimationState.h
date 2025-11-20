#pragma once

namespace kx {

struct HealthBarAnimationState {
    float healthBarFadeAlpha = 1.0f;

    float damageAccumulatorPercent = 0.0f;
    float damageAccumulatorAlpha = 1.0f;

    float damageNumberToDisplay = 0.0f;
    float damageNumberAlpha = 0.0f;
    float damageNumberYOffset = 0.0f;

    float healOverlayStartPercent = 0.0f;
    float healOverlayEndPercent = 0.0f;
    float healOverlayAlpha = 0.0f;

    float damageFlashAlpha = 0.0f;
    float damageFlashStartPercent = 0.0f;
    float healFlashAlpha = 0.0f;

    float animatedBarrier = 0.0f;

    float deathBurstAlpha = 0.0f;
    float deathBurstWidth = 0.0f;
};

} // namespace kx


