#include "VisualsFeature.h"
#include "UI/PlayersTab.h"
#include "UI/NpcsTab.h"
#include "UI/ObjectsTab.h"
#include "../../Core/Architecture/ServiceContext.h"
#include "../../Core/Services/EntityManager.h"
#include "../../Game/Services/Camera/Camera.h"
#include "../../Game/Services/Mumble/MumbleLinkManager.h"
#include "../../Utils/DebugLogger.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

VisualsFeature::VisualsFeature() 
    : m_masterRenderer(std::make_unique<MasterRenderer>()) {
}

bool VisualsFeature::Initialize(const ServiceContext& ctx) {
    LOG_INFO("VisualsFeature: Initializing...");
    
    // Store pointers to services
    m_entityManager = ctx.entities;
    m_camera = ctx.camera;
    
    // MasterRenderer initializes in its constructor, no additional setup needed
    return true;
}

void VisualsFeature::Shutdown() {
}

void VisualsFeature::Update(float deltaTime, const FrameGameData& frameData, const ServiceContext& ctx) {
    // Push configuration to Core service (Feature depends on Core - Allowed)
    if (m_entityManager) {
        m_entityManager->GetCombatStateManager().SetMaxTrailPoints(m_settings.playerESP.trails.maxPoints);
    }
}

void VisualsFeature::RenderDrawList(ImDrawList* drawList, const ServiceContext& ctx) {
    if (!m_masterRenderer || !m_camera || !m_entityManager) {
        return;
    }

    // Access MumbleLink data from context
    const MumbleLinkData* mumbleData = ctx.mumble ? ctx.mumble->GetData() : nullptr;
    
    if (!mumbleData) {
        return;
    }

    // Get display size from ImGui
    ImGuiIO& io = ImGui::GetIO();
    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;

    // Render ESP to the background draw list, passing our local settings
    m_masterRenderer->Render(screenWidth, screenHeight, mumbleData, *m_camera, *m_entityManager, m_settings);
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
