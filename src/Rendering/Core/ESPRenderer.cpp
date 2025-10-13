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
#include "ESPStateFinalizer.h"
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
static PooledFrameRenderData s_cachedFilteredData; // Filtered data ready for rendering
static float s_lastUpdateTime = 0.0f;
static CombatStateManager g_combatStateManager;
static uint64_t s_lastCleanupTime = 0;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

void ESPRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData) {
    if (!s_camera || ShouldHideESP(mumbleData)) {
        return;
    }

    // Get the timestamp ONCE for the entire frame.
    const uint64_t now = GetTickCount64();
    const float currentTimeSeconds = now / 1000.0f;

    ::ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    const auto& settings = AppState::Get().GetSettings();
    float espUpdateInterval = 1.0f / std::max(1.0f, settings.espUpdateRate);

    if (currentTimeSeconds - s_lastUpdateTime >= espUpdateInterval) {
        // Reset object pools to reuse all objects for this frame
        s_playerPool.Reset();
        s_npcPool.Reset();
        s_gadgetPool.Reset();
        s_cachedFilteredData.Reset();
        
        // Stage 1: Extract all data directly into object pools (eliminates heap allocations)
        PooledFrameRenderData extractedData;
        ESPDataExtractor::ExtractFrameData(s_playerPool, s_npcPool, s_gadgetPool, extractedData);

        // Stage 1.5: Update combat state manager with all extracted entities before filtering
        std::vector<RenderableEntity*> allEntities;
        allEntities.reserve(extractedData.players.size() + extractedData.npcs.size() + extractedData.gadgets.size());
        allEntities.insert(allEntities.end(), extractedData.players.begin(), extractedData.players.end());
        allEntities.insert(allEntities.end(), extractedData.npcs.begin(), extractedData.npcs.end());
        allEntities.insert(allEntities.end(), extractedData.gadgets.begin(), extractedData.gadgets.end());
        
        // Pass 'now' to the state manager
        g_combatStateManager.Update(allEntities, now);
        
        // Stage 2: Filter the pooled data (safe, configurable operations)  
        // Pass 'now' to the filter stage
        ESPFilter::FilterPooledData(extractedData, *s_camera, s_cachedFilteredData, g_combatStateManager, now);

        // NEW Stage 2.5: Finalize layout-dependent combat state
        // This calculates final health bar widths and runs the PostUpdate logic
        // for damage chunking before the rendering stage begins.
        ESPStateFinalizer::Finalize(s_cachedFilteredData, *s_camera, g_combatStateManager, now);

        // Stage 2.75: Update adaptive far plane (no change in its name)
        AppState::Get().UpdateAdaptiveFarPlane(s_cachedFilteredData);
        
        s_lastUpdateTime = currentTimeSeconds;
    }

    // Periodic cleanup of the combat state manager
    // Pass 'now' to the cleanup logic
    if (now - s_lastCleanupTime > 5000) {
        g_combatStateManager.Cleanup(now); // Pass 'now' here
        s_lastCleanupTime = now;
    }

    // Stage 3: Always render using cached filtered data (safe, fast operation)
    // Pass 'now' to the rendering stage
    ESPStageRenderer::RenderFrameData(drawList, screenWidth, screenHeight, s_cachedFilteredData, *s_camera,
                                      g_combatStateManager, now);
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx