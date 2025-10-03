#define NOMINMAX

#include "ESPRenderer.h"
#include "../Data/ESPData.h"

#include <algorithm>
#include <Windows.h>

#include "../../Core/AppState.h"
#include "../../Utils/SafeIterators.h"
#include "../../Utils/ObjectPool.h"
#include "../Utils/ESPMath.h"
#include "../Data/RenderableData.h"
#include "ESPDataExtractor.h"
#include "ESPFilter.h"
#include "ESPStageRenderer.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

// Object pools to eliminate heap churn - pre-allocate reasonable number of entities
static ObjectPool<RenderablePlayer> s_playerPool(500);
static ObjectPool<RenderableNpc> s_npcPool(2000);
static ObjectPool<RenderableGadget> s_gadgetPool(5000);

// Static variables for frame rate limiting and three-stage pipeline
static PooledFrameRenderData s_cachedFilteredData; // Filtered data ready for rendering
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
        // Reset object pools to reuse all objects for this frame
        s_playerPool.Reset();
        s_npcPool.Reset();
        s_gadgetPool.Reset();
        s_cachedFilteredData.Reset();
        
        // Stage 1: Extract all data directly into object pools (eliminates heap allocations)
        PooledFrameRenderData extractedData;
        ESPDataExtractor::ExtractFrameData(s_playerPool, s_npcPool, s_gadgetPool, extractedData);
        
        // Stage 2: Filter the pooled data (safe, configurable operations)  
        ESPFilter::FilterPooledData(extractedData, *s_camera, s_cachedFilteredData);
        
        // Stage 2.5: Update adaptive far plane for "No Limit" mode (once per second internally)
        AppState::Get().UpdateAdaptiveFarPlane(s_cachedFilteredData);
        
        s_lastUpdateTime = currentTime;
    }
    
    // Stage 3: Always render using cached filtered data (safe, fast operation)
    ESPStageRenderer::RenderFrameData(drawList, screenWidth, screenHeight, s_cachedFilteredData, *s_camera);
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx