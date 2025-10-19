#define NOMINMAX

#include "ESPRenderer.h"
#include "../Data/ESPData.h"

#include <algorithm>
#include <Windows.h>

#include "../../Core/AppState.h"
#include "../../Utils/ObjectPool.h"
#include "../Utils/ESPMath.h"
#include "../Data/RenderableData.h"
#include "ESPDataExtractor.h"
#include "ESPFilter.h"
#include "ESPVisualsProcessor.h"
#include "ESPStageRenderer.h"
#include "../Combat/CombatStateManager.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

// Object pools to eliminate heap churn - pre-allocate reasonable number of entities
static ObjectPool<RenderablePlayer> s_playerPool(500);
static ObjectPool<RenderableNpc> s_npcPool(2000);
static ObjectPool<RenderableGadget> s_gadgetPool(5000);

// Static variables for frame rate limiting and three-stage pipeline
static PooledFrameRenderData s_processedRenderData; // Filtered data ready for rendering
static float s_lastUpdateTime = 0.0f;
static CombatStateManager g_combatStateManager;
static uint64_t s_lastCleanupTime = 0;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

void ESPRenderer::UpdateESPData(const FrameContext& frameContext, float currentTimeSeconds) {
    float espUpdateInterval = 1.0f / std::max(1.0f, frameContext.settings.espUpdateRate);

    if (currentTimeSeconds - s_lastUpdateTime >= espUpdateInterval) {
        // Reset object pools to reuse all objects for this frame
        s_playerPool.Reset();
        s_npcPool.Reset();
        s_gadgetPool.Reset();
        s_processedRenderData.Reset();
        
        // Stage 1: Extract
        PooledFrameRenderData extractedData;
        ESPDataExtractor::ExtractFrameData(s_playerPool, s_npcPool, s_gadgetPool, extractedData);
        
        // Stage 1.5: Update combat state
        std::vector<RenderableEntity*> allEntities;
        allEntities.reserve(extractedData.players.size() + extractedData.npcs.size() + extractedData.gadgets.size());
        allEntities.insert(allEntities.end(), extractedData.players.begin(), extractedData.players.end());
        allEntities.insert(allEntities.end(), extractedData.npcs.begin(), extractedData.npcs.end());
        allEntities.insert(allEntities.end(), extractedData.gadgets.begin(), extractedData.gadgets.end());
        g_combatStateManager.Update(allEntities, frameContext.now);
        
        // Stage 2: Filter
        PooledFrameRenderData filteredData;
        ESPFilter::FilterPooledData(extractedData, *s_camera, filteredData, g_combatStateManager, frameContext.now);
        
        // Stage 2.5: Calculate Visuals
        ESPVisualsProcessor::Process(frameContext, filteredData, s_processedRenderData);

        // Stage 2.8: Update adaptive far plane (use extracted data for true scene depth)
        AppState::Get().UpdateAdaptiveFarPlane(extractedData);
        
        s_lastUpdateTime = currentTimeSeconds;
    }
}

void ESPRenderer::HandlePeriodicCleanup(uint64_t now) {
    if (now - s_lastCleanupTime > 5000) { // 5000ms interval
        g_combatStateManager.Cleanup(now);
        s_lastCleanupTime = now;
    }
}

void ESPRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData) {
    if (!s_camera || ShouldHideESP(mumbleData)) {
        return;
    }

    // Critical: Check if ImGui context is still valid before any ImGui operations
    if (!ImGui::GetCurrentContext()) {
        return;
    }

    const uint64_t now = GetTickCount64();
    const float currentTimeSeconds = now / 1000.0f;

    // 1. Create the context for the current frame
    FrameContext frameContext = {
        now,
        *s_camera,
        g_combatStateManager,
        AppState::Get().GetSettings(),
        ImGui::GetBackgroundDrawList(),
        screenWidth,
        screenHeight
    };

    // 2. Run the low-frequency logic/update pipeline if needed
    UpdateESPData(frameContext, currentTimeSeconds);

    // 3. Handle periodic maintenance tasks
    HandlePeriodicCleanup(now);

    // 4. Render the final, processed data every frame
    ESPStageRenderer::RenderFrameData(frameContext, s_processedRenderData);
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx