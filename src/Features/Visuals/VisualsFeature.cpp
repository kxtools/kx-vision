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

void VisualsFeature::Update(float deltaTime, const FrameGameData& frameData) {
    // Currently no per-frame update logic needed
    // Frame data is now available if needed in the future
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
    try {
        if (j.contains("visuals")) {
            m_settings = j["visuals"].get<VisualsConfiguration>();
            LOG_INFO("VisualsFeature: Settings loaded successfully");
        } else {
            LOG_WARN("VisualsFeature: No 'visuals' key found in settings, using defaults");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("VisualsFeature: Failed to load settings: %s", e.what());
    }
}

void VisualsFeature::SaveSettings(nlohmann::json& j) {
    try {
        j["visuals"] = m_settings;
        LOG_DEBUG("VisualsFeature: Settings saved successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("VisualsFeature: Failed to save settings: %s", e.what());
    }
}

} // namespace kx
