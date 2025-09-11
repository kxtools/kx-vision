#define NOMINMAX

#include "ESPRenderer.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <gtc/type_ptr.hpp>
#include <Windows.h>

#include "../Core/Config.h"
#include "../Core/AppState.h"
#include "../Utils/SafeIterators.h"
#include "ESPMath.h"
#include "ESPData.h"
#include "ESPStyling.h"
#include "ESPFormatting.h"
#include "RenderableData.h"
#include "ESPDataExtractor.h"
#include "ESPStageRenderer.h"
#include "../../libs/ImGui/imgui.h"
#include "../Game/AddressManager.h"
#include "../Game/ReClassStructs.h"
#include "../Utils/EntityFilter.h"

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

// Static variables for frame rate limiting
static FrameRenderData s_cachedFrameData;
static float s_lastUpdateTime = 0.0f;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

void ESPRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData) {
    if (!s_camera || ShouldHideESP(mumbleData)) {
        return;
    }

    ::ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Get current time in seconds
    float currentTime = GetTickCount64() / 1000.0f;
    
    // Get ESP update interval from settings (default 60 FPS)
    const auto& settings = AppState::Get().GetSettings();
    float espUpdateInterval = 1.0f / std::max(1.0f, settings.espUpdateRate);
    
    // Only update ESP data at limited frame rate
    if (currentTime - s_lastUpdateTime >= espUpdateInterval) {
        // Stage 1: Extract all data from game memory (unsafe operations) - only when needed
        ESPDataExtractor::ExtractFrameData(s_cachedFrameData);
        s_lastUpdateTime = currentTime;
    }
    
    // Stage 2: Always render using cached data (safe, fast operation)
    ESPStageRenderer::RenderFrameData(drawList, screenWidth, screenHeight, s_cachedFrameData, *s_camera);
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx