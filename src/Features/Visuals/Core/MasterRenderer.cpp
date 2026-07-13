#define NOMINMAX

#include "MasterRenderer.h"
#include "../../../Game/Data/FrameData.h"

#include <algorithm>
#include <Windows.h>

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
                            Camera& camera, const FrameSnapshotHandle& snapshot,
                            const VisualCombatTracker::StateMap& visualCombatStates,
                            const VisualsConfiguration& visualsConfig,
                            float adaptiveFarPlane) {
    if (ShouldHideESP(mumbleData)) {
        return;
    }

    if (!ImGui::GetCurrentContext()) {
        return;
    }

    const uint64_t now = GetTickCount64();
    bool isInWvW = mumbleData && mumbleData->context.mapType == 18; // WvW map type

    if (!snapshot) return;

    // Guardrail: observations belong to same immutable generation as entities. Render
    // must not reach back into game-thread tracker state.
    FrameContext frameContext = {
        now,
        camera,
        snapshot->combatObservations,
        visualCombatStates,
        visualsConfig,
        adaptiveFarPlane,
        ImGui::GetBackgroundDrawList(),
        screenWidth,
        screenHeight,
        isInWvW
    };

    // Filter the global data for this frame
    FilterAndProcessData(snapshot->frameData, frameContext, visualsConfig);

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
