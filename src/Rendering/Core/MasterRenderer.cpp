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
        
        m_extractionData.Reset();
        DataExtractor::ExtractFrameData(m_playerPool, m_npcPool, m_gadgetPool, m_attackTargetPool, m_extractionData);
        
        size_t totalCount = m_extractionData.players.size() + m_extractionData.npcs.size() + 
                            m_extractionData.gadgets.size() + m_extractionData.attackTargets.size();
        m_activeKeys.clear();
        m_activeKeys.reserve(totalCount);
        
        auto collectKeys = [&](const auto& collection) {
            for (const auto* e : collection) {
                m_activeKeys.insert(e->GetCombatKey());
            }
        };

        collectKeys(m_extractionData.players);
        collectKeys(m_extractionData.npcs);
        collectKeys(m_extractionData.gadgets);
        collectKeys(m_extractionData.attackTargets);

        m_combatStateManager.Prune(m_activeKeys);
        
        std::vector<RenderableEntity*> allEntities;
        allEntities.reserve(totalCount);
        allEntities.insert(allEntities.end(), m_extractionData.players.begin(), m_extractionData.players.end());
        allEntities.insert(allEntities.end(), m_extractionData.npcs.begin(), m_extractionData.npcs.end());
        allEntities.insert(allEntities.end(), m_extractionData.gadgets.begin(), m_extractionData.gadgets.end());
        allEntities.insert(allEntities.end(), m_extractionData.attackTargets.begin(), m_extractionData.attackTargets.end());
        m_combatStateManager.Update(allEntities, frameContext.now);
        
        EntityFilter::FilterPooledData(m_extractionData, frameContext, m_processedRenderData);

        AppState::Get().UpdateAdaptiveFarPlane(m_extractionData);
        
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
    m_extractionData.Reset();
    m_activeKeys.clear();
}

bool MasterRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx