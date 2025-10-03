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

// Persistent entity state for interpolation
std::unordered_map<const void*, RenderablePlayer> ESPRenderer::s_playerData;
std::unordered_map<const void*, RenderableNpc> ESPRenderer::s_npcData;
std::unordered_map<const void*, RenderableGadget> ESPRenderer::s_gadgetData;

// Object pools to eliminate heap churn - pre-allocate reasonable number of entities
static ObjectPool<RenderablePlayer> s_playerPool(500);
static ObjectPool<RenderableNpc> s_npcPool(2000);
static ObjectPool<RenderableGadget> s_gadgetPool(5000);

// Static variables for frame rate limiting and three-stage pipeline
static PooledFrameRenderData s_cachedFilteredData; // Filtered data ready for rendering
static float s_lastUpdateTime = 0.0f;
static float s_lastCleanupTime = 0.0f; // Track when we last cleaned up stale entities

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
        
        // Periodic cleanup of stale entities (every 5 seconds)
        if (currentTime - s_lastCleanupTime >= 5.0f) {
            CleanupStaleEntities();
            s_lastCleanupTime = currentTime;
        }
    }
    
    // Stage 3: Always render using cached filtered data (safe, fast operation)
    ESPStageRenderer::RenderFrameData(drawList, screenWidth, screenHeight, s_cachedFilteredData, *s_camera);
}

void ESPRenderer::CleanupStaleEntities() {
    double currentTime = GetTickCount64() / 1000.0;
    constexpr double STALE_THRESHOLD = 5.0; // Remove entities not seen in 5 seconds
    
    // Clean up players
    for (auto it = s_playerData.begin(); it != s_playerData.end(); ) {
        if (currentTime - it->second.lastUpdateTime > STALE_THRESHOLD) {
            it = s_playerData.erase(it);
        } else {
            ++it;
        }
    }
    
    // Clean up NPCs
    for (auto it = s_npcData.begin(); it != s_npcData.end(); ) {
        if (currentTime - it->second.lastUpdateTime > STALE_THRESHOLD) {
            it = s_npcData.erase(it);
        } else {
            ++it;
        }
    }
    
    // Clean up gadgets
    for (auto it = s_gadgetData.begin(); it != s_gadgetData.end(); ) {
        if (currentTime - it->second.lastUpdateTime > STALE_THRESHOLD) {
            it = s_gadgetData.erase(it);
        } else {
            ++it;
        }
    }
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx