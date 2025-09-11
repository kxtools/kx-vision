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
#include "ESP_Helpers.h"
#include "ESPData.h"
#include "EnhancedESPHelpers.h"
#include "StringHelpers.h"
#include "RenderableData.h"
#include "ESPDataExtractor.h"
#include "ESPStageRenderer.h"
#include "../../libs/ImGui/imgui.h"
#include "../Game/AddressManager.h"
#include "../Game/GameStructs.h"
#include "../Game/ReClassStructs.h"
#include "../Utils/EntityFilter.h"

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

void ESPRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData) {
    if (!s_camera || ShouldHideESP(mumbleData)) {
        return;
    }

    ::ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Two-stage rendering pipeline for improved stability
    FrameRenderData frameData;
    
    // Stage 1: Extract all data from game memory (unsafe operations)
    ESPDataExtractor::ExtractFrameData(frameData);
    
    // Stage 2: Render using safe local data (no game memory access)
    ESPStageRenderer::RenderFrameData(drawList, screenWidth, screenHeight, frameData, *s_camera);
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx