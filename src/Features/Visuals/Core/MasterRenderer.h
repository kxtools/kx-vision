#pragma once

#include "../../../Game/Services/Camera/Camera.h"
#include "../../../Game/Services/Mumble/MumbleLink.h"
#include "../../../Game/Data/EntityData.h"
#include "../../../Game/Data/FrameData.h"
#include "../../../Rendering/Shared/LayoutConstants.h"
#include "../Settings/VisualsSettings.h"
#include <vector>

namespace kx {

class MasterRenderer {
public:
    MasterRenderer();
    ~MasterRenderer() = default;

    void Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData, Camera& camera, const VisualsConfiguration& visualsConfig);

    void Reset();

private:
    bool ShouldHideESP(const MumbleLinkData* mumbleData);
    
    /**
     * @brief Filters raw entity data based on visibility and distance settings.
     * This runs every frame on the global data from AppLifecycleManager.
     * @param extractionData The raw entity data from AppLifecycleManager
     * @param context The current frame's context.
     * @param visualsConfig Feature-specific visuals configuration
     */
    void FilterAndProcessData(const FrameGameData& extractionData, const FrameContext& context, const VisualsConfiguration& visualsConfig);

    // Filtered render data (only what should be displayed)
    FrameGameData m_processedRenderData;
};

} // namespace kx

