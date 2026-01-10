#include "VisualsFeature.h"
#include "UI/PlayersTab.h"
#include "UI/NpcsTab.h"
#include "UI/ObjectsTab.h"
#include "../../Core/AppLifecycleManager.h"
#include "../../../libs/ImGui/imgui.h"
#include <spdlog/spdlog.h>

namespace kx {

VisualsFeature::VisualsFeature() 
    : m_masterRenderer(std::make_unique<MasterRenderer>()) {
}

bool VisualsFeature::Initialize() {
    spdlog::info("VisualsFeature: Initializing...");
    // MasterRenderer initializes in its constructor, no additional setup needed
    return true;
}

void VisualsFeature::Update(float deltaTime) {
    // MasterRenderer updates are handled during Render() call
    // No per-frame logic needed here
}

void VisualsFeature::RenderDrawList(ImDrawList* drawList) {
    if (!m_masterRenderer) {
        return;
    }

    // Access frame context from AppLifecycleManager
    Camera& camera = g_App.GetCamera();
    const MumbleLinkData* mumbleData = g_App.GetMumbleLinkData();
    
    if (!mumbleData) {
        return;
    }

    // Get display size from ImGui
    ImGuiIO& io = ImGui::GetIO();
    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;

    // Render ESP to the background draw list
    m_masterRenderer->Render(screenWidth, screenHeight, mumbleData, camera);
}

void VisualsFeature::OnMenuRender() {
    // Render ESP configuration tabs
    kx::GUI::RenderPlayersTab();
    kx::GUI::RenderNPCsTab();
    kx::GUI::RenderObjectsTab();
}

} // namespace kx
