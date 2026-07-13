#pragma once

namespace kx::CoordinateTransform {

// GW2 internal coordinates use a different scale than MumbleLink meters.
inline constexpr float GAME_TO_MUMBLE_SCALE_FACTOR = 1.23f;

} // namespace kx::CoordinateTransform
