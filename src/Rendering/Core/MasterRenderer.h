#pragma once

#include "../../Game/Camera.h"
#include "../../Game/MumbleLink.h"
#include "Data/FrameData.h"

namespace kx {

class MasterRenderer {
public:
    static void Initialize(Camera& camera);
    static void Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData);

private:
    static bool ShouldHideESP(const MumbleLinkData* mumbleData);
    
    /**
     * @brief Executes the low-frequency data processing pipeline if the update interval has passed.
     * This includes data extraction, combat state updates, filtering, and visual processing.
     * @param context The current frame's context.
     * @param currentTimeSeconds The current time in seconds, used to check the update interval.
     */
    static void UpdateESPData(const FrameContext& context, float currentTimeSeconds);

    static Camera* s_camera; // Camera reference for world-to-screen projections
};

} // namespace kx

