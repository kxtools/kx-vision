#define NOMINMAX

#include "MasterRenderer.h"
#include "../../../Game/Data/FrameData.h"

#include <algorithm>
#include <Windows.h>

#include "../../../Core/AppState.h"
#include "../../../Core/AppLifecycleManager.h"
#include "StageRenderer.h"
#include "../../../../libs/ImGui/imgui.h"
#include "../Logic/EntityFilter.h"

namespace kx {

MasterRenderer::MasterRenderer() {
}

void MasterRenderer::FilterAndProcessData(const FrameGameData& extractionData, const FrameContext& context, const VisualsConfiguration& visualsConfig) {
    m_processedRenderData.Reset();
    EntityFilter::FilterPooledData(extractionData, context, visualsConfig, m_processedRenderData);
}

void MasterRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData, Camera& camera, const VisualsConfiguration& visualsConfig) {
    if (ShouldHideESP(mumbleData)) {
        return;
    }

    if (!ImGui::GetCurrentContext()) {
        return;
    }

    const uint64_t now = GetTickCount64();
    bool isInWvW = g_App.GetMumbleLinkManager().isInWvW();

    // Get globally extracted data from EntityManager
    const FrameGameData& extractionData = g_App.GetEntityManager().GetFrameData();
    CombatStateManager& combatStateManager = g_App.GetEntityManager().GetCombatStateManager();

    FrameContext frameContext = {
        now,
        camera,
        combatStateManager,
        AppState::Get().GetSettings(),
        ImGui::GetBackgroundDrawList(),
        screenWidth,
        screenHeight,
        isInWvW
    };

    // Filter the global data for this frame
    FilterAndProcessData(extractionData, frameContext, visualsConfig);

    // Render the filtered data
    StageRenderer::RenderFrameData(frameContext, m_processedRenderData, visualsConfig);
}

void MasterRenderer::Reset() {
    m_processedRenderData.Reset();
}

bool MasterRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx