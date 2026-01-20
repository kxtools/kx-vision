#include "VisualsFeature.h"
#include "UI/PlayersTab.h"
#include "UI/NpcsTab.h"
#include "UI/ObjectsTab.h"
#include "../../Core/AppLifecycleManager.h"
#include "../../Utils/DebugLogger.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

VisualsFeature::VisualsFeature() 
    : m_masterRenderer(std::make_unique<MasterRenderer>()) {
}

bool VisualsFeature::Initialize() {
    LOG_INFO("VisualsFeature: Initializing...");
    // MasterRenderer initializes in its constructor, no additional setup needed
    return true;
}

void VisualsFeature::Shutdown() {
}

void VisualsFeature::Update(float deltaTime, const FrameGameData& frameData) {
    // Push configuration to Core service (Feature depends on Core - Allowed)
    g_App.GetEntityManager().GetCombatStateManager().SetMaxTrailPoints(m_settings.playerESP.trails.maxPoints);
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

    // Render ESP to the background draw list, passing our local settings
    m_masterRenderer->Render(screenWidth, screenHeight, mumbleData, camera, m_settings);
}

void VisualsFeature::OnMenuRender() {
    // Render ESP configuration tabs
    kx::GUI::RenderPlayersTab(m_settings);
    kx::GUI::RenderNPCsTab(m_settings);
    kx::GUI::RenderObjectsTab(m_settings);
}

void VisualsFeature::LoadSettings(const nlohmann::json& j) {
    if (j.contains(SettingsKey)) {
        m_settings = j[SettingsKey].get<VisualsConfiguration>();
        LOG_INFO("Visuals settings loaded");
    }
}

void VisualsFeature::SaveSettings(nlohmann::json& j) {
    j[SettingsKey] = m_settings;
}

} // namespace kx
