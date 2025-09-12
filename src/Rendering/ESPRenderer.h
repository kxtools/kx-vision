#pragma once

#include "../Game/Camera.h"
#include "../Game/MumbleLink.h"

namespace kx {

class ESPRenderer {
public:
    static void Initialize(Camera& camera);
    static void Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData);

private:
    static bool ShouldHideESP(const MumbleLinkData* mumbleData);
    static Camera* s_camera; // Camera reference for world-to-screen projections
};

} // namespace kx

