#define NOMINMAX

#include "ESPRenderer.h"
#include "../Data/ESPData.h"

#include <algorithm>
#include <unordered_set>
#include <Windows.h>

#include "../../Core/AppState.h"
#include "../../Core/AppLifecycleManager.h"
#include "../../Utils/ObjectPool.h"
#include "../Data/RenderableData.h"
#include "../Extraction/ESPDataExtractor.h"
#include "ESPVisualsProcessor.h"
#include "ESPStageRenderer.h"
#include "../Combat/CombatStateManager.h"
#include "../../../libs/ImGui/imgui.h"
#include "Logic/EntityFilter.h"

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

// Object pools to eliminate heap churn - pre-allocate reasonable number of entities
static ObjectPool<RenderablePlayer> s_playerPool(500);
static ObjectPool<RenderableNpc> s_npcPool(2000);
static ObjectPool<RenderableGadget> s_gadgetPool(5000);
static ObjectPool<RenderableAttackTarget> s_attackTargetPool(1000);

// Static variables for frame rate limiting and three-stage pipeline
static PooledFrameRenderData s_processedRenderData; // Filtered data ready for rendering
static float s_lastUpdateTime = 0.0f;
static CombatStateManager g_combatStateManager;

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
        s_attackTargetPool.Reset();
        s_processedRenderData.Reset();
        
        // Stage 1: Extract
        PooledFrameRenderData extractedData;
        ESPDataExtractor::ExtractFrameData(s_playerPool, s_npcPool, s_gadgetPool, s_attackTargetPool, extractedData);
        
        // Build a set of all currently active entity addresses
        std::unordered_set<const void*> activeEntities;
        for (auto* e : extractedData.players) { activeEntities.insert(e->address); }
        for (auto* e : extractedData.npcs) { activeEntities.insert(e->address); }
        for (auto* e : extractedData.gadgets) { activeEntities.insert(e->address); }
        for (auto* e : extractedData.attackTargets) { activeEntities.insert(e->address); }

        // Tell the CombatStateManager to remove any state for entities that no longer exist.
        g_combatStateManager.Prune(activeEntities);
        
        // Stage 1.5: Update combat state
        std::vector<RenderableEntity*> allEntities;
        allEntities.reserve(extractedData.players.size() + extractedData.npcs.size() + extractedData.gadgets.size() + extractedData.attackTargets.size());
        allEntities.insert(allEntities.end(), extractedData.players.begin(), extractedData.players.end());
        allEntities.insert(allEntities.end(), extractedData.npcs.begin(), extractedData.npcs.end());
        allEntities.insert(allEntities.end(), extractedData.gadgets.begin(), extractedData.gadgets.end());
        allEntities.insert(allEntities.end(), extractedData.attackTargets.begin(), extractedData.attackTargets.end());
        g_combatStateManager.Update(allEntities, frameContext.now);
        
        // Stage 2: Filter
        PooledFrameRenderData filteredData;
        EntityFilter::FilterPooledData(extractedData, frameContext, filteredData);
        
        // Stage 2.5: Calculate Visuals
        ESPVisualsProcessor::Process(frameContext, filteredData, s_processedRenderData);

        // Stage 2.8: Update adaptive far plane (use extracted data for true scene depth)
        AppState::Get().UpdateAdaptiveFarPlane(extractedData);
        
        s_lastUpdateTime = currentTimeSeconds;
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

    // Get the WvW state from the single source of truth.
    bool isInWvW = g_App.GetMumbleLinkManager().isInWvW();

    // 1. Create the context for the current frame
    FrameContext frameContext = {
        now,
        *s_camera,
        g_combatStateManager,
        AppState::Get().GetSettings(),
        ImGui::GetBackgroundDrawList(),
        screenWidth,
        screenHeight,
        isInWvW
    };

    // 2. Run the low-frequency logic/update pipeline if needed
    UpdateESPData(frameContext, currentTimeSeconds);

    // 3. Render the final, processed data every frame
    ESPStageRenderer::RenderFrameData(frameContext, s_processedRenderData);
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx