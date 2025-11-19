#define NOMINMAX

#include "MasterRenderer.h"
#include "../Data/FrameData.h"

#include <algorithm>
#include <ankerl/unordered_dense.h>
#include <Windows.h>

#include "../../Core/AppState.h"
#include "../../Core/AppLifecycleManager.h"
#include "../../Utils/ObjectPool.h"
#include "../Data/RenderableData.h"
#include "../Extraction/DataExtractor.h"
#include "StageRenderer.h"
#include "../Combat/CombatStateManager.h"
#include "../Combat/CombatStateKey.h"
#include "../../../libs/ImGui/imgui.h"
#include "Logic/EntityFilter.h"
#include "Logic/FrameDataProcessor.h"

namespace kx {

MasterRenderer::MasterRenderer() {
}

void MasterRenderer::UpdateESPData(const FrameContext& frameContext, float currentTimeSeconds) {
    float espUpdateInterval = 1.0f / std::max(1.0f, frameContext.settings.espUpdateRate);

    if (currentTimeSeconds - m_lastUpdateTime >= espUpdateInterval) {
        m_playerPool.Reset();
        m_npcPool.Reset();
        m_gadgetPool.Reset();
        m_attackTargetPool.Reset();
        m_processedRenderData.Reset();
        
        PooledFrameRenderData extractedData;
        DataExtractor::ExtractFrameData(m_playerPool, m_npcPool, m_gadgetPool, m_attackTargetPool, extractedData);
        
        ankerl::unordered_dense::set<CombatStateKey> activeKeys;
        size_t totalCount = extractedData.players.size() + extractedData.npcs.size() + 
                            extractedData.gadgets.size() + extractedData.attackTargets.size();
        activeKeys.reserve(totalCount);
        
        auto collectKeys = [&](const auto& collection) {
            for (const auto* e : collection) {
                activeKeys.insert(e->GetCombatKey());
            }
        };

        collectKeys(extractedData.players);
        collectKeys(extractedData.npcs);
        collectKeys(extractedData.gadgets);
        collectKeys(extractedData.attackTargets);

        m_combatStateManager.Prune(activeKeys);
        
        std::vector<RenderableEntity*> allEntities;
        allEntities.reserve(extractedData.players.size() + extractedData.npcs.size() + extractedData.gadgets.size() + extractedData.attackTargets.size());
        allEntities.insert(allEntities.end(), extractedData.players.begin(), extractedData.players.end());
        allEntities.insert(allEntities.end(), extractedData.npcs.begin(), extractedData.npcs.end());
        allEntities.insert(allEntities.end(), extractedData.gadgets.begin(), extractedData.gadgets.end());
        allEntities.insert(allEntities.end(), extractedData.attackTargets.begin(), extractedData.attackTargets.end());
        m_combatStateManager.Update(allEntities, frameContext.now);
        
        PooledFrameRenderData filteredData;
        EntityFilter::FilterPooledData(extractedData, frameContext, filteredData);
        
        FrameDataProcessor::Process(frameContext, filteredData, m_processedRenderData);

        AppState::Get().UpdateAdaptiveFarPlane(extractedData);
        
        m_lastUpdateTime = currentTimeSeconds;
    }
}


void MasterRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData, Camera& camera) {
    if (ShouldHideESP(mumbleData)) {
        return;
    }

    if (!ImGui::GetCurrentContext()) {
        return;
    }

    const uint64_t now = GetTickCount64();
    const float currentTimeSeconds = now / 1000.0f;

    bool isInWvW = g_App.GetMumbleLinkManager().isInWvW();

    FrameContext frameContext = {
        now,
        camera,
        m_combatStateManager,
        AppState::Get().GetSettings(),
        ImGui::GetBackgroundDrawList(),
        screenWidth,
        screenHeight,
        isInWvW
    };

    UpdateESPData(frameContext, currentTimeSeconds);

    StageRenderer::RenderFrameData(frameContext, m_processedRenderData);
}

void MasterRenderer::Reset() {
    m_playerPool.Reset();
    m_npcPool.Reset();
    m_gadgetPool.Reset();
    m_attackTargetPool.Reset();
    m_processedRenderData.Reset();
}

bool MasterRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx