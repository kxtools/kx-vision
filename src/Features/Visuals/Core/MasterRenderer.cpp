#define NOMINMAX

#include "MasterRenderer.h"
#include "../../../Game/Data/FrameData.h"

#include <algorithm>
#include <Windows.h>

#include "../../../Core/AppState.h"
#include "../../../Core/Services/EntityManager.h"
#include "../../../Game/Services/Mumble/MumbleLinkManager.h"
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

void MasterRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData, 
                            Camera& camera, EntityManager& entityManager, const VisualsConfiguration& visualsConfig) {
    if (ShouldHideESP(mumbleData)) {
        return;
    }

    if (!ImGui::GetCurrentContext()) {
        return;
    }

    const uint64_t now = GetTickCount64();
    bool isInWvW = mumbleData && mumbleData->context.mapType == 18; // WvW map type

    // Get globally extracted data from EntityManager
    const FrameGameData& extractionData = entityManager.GetFrameData();
    CombatStateManager& combatStateManager = entityManager.GetCombatStateManager();

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