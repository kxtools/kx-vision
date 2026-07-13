#include "VisualsFeature.h"
#include "UI/PlayersTab.h"
#include "UI/NpcsTab.h"
#include "UI/ObjectsTab.h"
#include "UI/AppearanceTab.h"
#include "../../Core/Architecture/ServiceContext.h"
#include "../../Core/Services/EntityManager.h"
#include "../../Game/Services/Camera/Camera.h"
#include "../../Game/Services/Mumble/MumbleLinkManager.h"
#include "../../Utils/DebugLogger.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>
#include <cmath>

namespace kx {

VisualsFeature::VisualsFeature()
    : FeatureWithSettings(SettingsSections::Visuals),
      m_masterRenderer(std::make_unique<MasterRenderer>()) {
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
    m_visualCombatTracker.Reset();
    if (m_masterRenderer) {
        m_masterRenderer->Reset();
    }
}

void VisualsFeature::Update(float deltaTime, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx) {
    const int maxTrailPoints = (std::clamp)(
        m_settings.playerESP.trails.maxPoints,
        0,
        static_cast<int>(MAX_TRAIL_HISTORY_CAPACITY));
    if (m_entityManager) {
        m_entityManager->SetUpdateRateHz(m_settings.espUpdateRate);
    }
    if (snapshot) {
        // Render-thread-owned presentation state consumes one immutable game snapshot.
        m_visualCombatTracker.Update(*snapshot, static_cast<size_t>(maxTrailPoints));
        // Guardrail: adaptive visual policy reads only the pinned immutable snapshot.
        // Moving this back to live entity/game memory would violate render-thread ownership.
        m_adaptiveFarPlane.UpdateAndGetFarPlane(snapshot->frameData);
    }
}

void VisualsFeature::RenderDrawList(ImDrawList* drawList, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx) {
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
    m_masterRenderer->Render(screenWidth, screenHeight, mumbleData, *m_camera, snapshot,
                             m_visualCombatTracker.GetStates(), m_settings,
                             m_adaptiveFarPlane.GetCurrentFarPlane());
}

void VisualsFeature::OnTabRender() {
    // Render ESP configuration tabs
    kx::GUI::RenderPlayersTab(m_settings);
    kx::GUI::RenderNPCsTab(m_settings);
    kx::GUI::RenderObjectsTab(m_settings);
    kx::GUI::RenderAppearanceTab(m_settings, m_adaptiveFarPlane.GetCurrentFarPlane());
}

void VisualsFeature::NormalizeSettings() {
    const auto finiteOr = [](float value, float fallback) { return (std::isfinite)(value) ? value : fallback; };
    auto& trails = m_settings.playerESP.trails;
    trails.maxPoints = (std::clamp)(trails.maxPoints, 0, static_cast<int>(MAX_TRAIL_HISTORY_CAPACITY));
    trails.maxDuration = (std::clamp)(finiteOr(trails.maxDuration, 1.0f), 0.0f, 60.0f);
    trails.thickness = (std::clamp)(finiteOr(trails.thickness, 2.0f), 0.5f, 10.0f);
    m_settings.playerESP.hostileBoostMultiplier = (std::clamp)(
        finiteOr(m_settings.playerESP.hostileBoostMultiplier, 2.0f), 0.0f, 10.0f);
    m_settings.objectESP.maxBoxHeight = (std::clamp)(finiteOr(m_settings.objectESP.maxBoxHeight, 10.0f), 0.0f, 1000.0f);
    m_settings.espUpdateRate = (std::clamp)(finiteOr(m_settings.espUpdateRate, 60.0f), 30.0f, 360.0f);
    m_settings.distance.renderDistanceLimit = (std::clamp)(finiteOr(m_settings.distance.renderDistanceLimit, 90.0f), 10.0f, 500.0f);
    m_settings.appearance.globalOpacity = (std::clamp)(finiteOr(m_settings.appearance.globalOpacity, 0.8f), 0.0f, 1.0f);
    m_settings.scaling.scalingStartDistance = (std::clamp)(finiteOr(m_settings.scaling.scalingStartDistance, 20.0f), 0.0f, 500.0f);
    m_settings.scaling.minScale = (std::clamp)(finiteOr(m_settings.scaling.minScale, 0.1f), 0.01f, 2.0f);
    m_settings.scaling.maxScale = (std::clamp)(finiteOr(m_settings.scaling.maxScale, 1.0f), m_settings.scaling.minScale, 5.0f);
    m_settings.scaling.limitDistanceFactor = (std::clamp)(finiteOr(m_settings.scaling.limitDistanceFactor, 110.0f), 1.0f, 1000.0f);
    m_settings.scaling.limitScalingExponent = (std::clamp)(finiteOr(m_settings.scaling.limitScalingExponent, 1.2f), 0.1f, 5.0f);
    m_settings.scaling.noLimitScalingExponent = (std::clamp)(finiteOr(m_settings.scaling.noLimitScalingExponent, 1.2f), 0.1f, 5.0f);
    m_settings.sizes.baseFontSize = (std::clamp)(finiteOr(m_settings.sizes.baseFontSize, 16.0f), 1.0f, 100.0f);
    m_settings.sizes.minFontSize = (std::clamp)(finiteOr(m_settings.sizes.minFontSize, 8.0f), 1.0f, m_settings.sizes.baseFontSize);
    m_settings.sizes.baseDotRadius = (std::clamp)(finiteOr(m_settings.sizes.baseDotRadius, 3.0f), 0.1f, 100.0f);
    m_settings.sizes.baseBoxThickness = (std::clamp)(finiteOr(m_settings.sizes.baseBoxThickness, 2.0f), 0.1f, 20.0f);
    m_settings.sizes.baseBoxHeight = (std::clamp)(finiteOr(m_settings.sizes.baseBoxHeight, 90.0f), 1.0f, 1000.0f);
    m_settings.sizes.baseBoxWidth = (std::clamp)(finiteOr(m_settings.sizes.baseBoxWidth, 45.0f), 1.0f, 1000.0f);
    m_settings.sizes.baseHealthBarWidth = (std::clamp)(finiteOr(m_settings.sizes.baseHealthBarWidth, 60.0f), 1.0f, 1000.0f);
    m_settings.sizes.baseHealthBarHeight = (std::clamp)(finiteOr(m_settings.sizes.baseHealthBarHeight, 7.0f), 1.0f, 100.0f);
    if (static_cast<int>(m_settings.distance.mode) < 0 || static_cast<int>(m_settings.distance.mode) > 3) m_settings.distance.mode = DistanceCullingMode::Natural;
    if (static_cast<int>(m_settings.distance.displayMode) < 0 || static_cast<int>(m_settings.distance.displayMode) > 2) m_settings.distance.displayMode = DistanceDisplayMode::Meters;
    if (static_cast<int>(trails.displayMode) < 0 || static_cast<int>(trails.displayMode) > 1) trails.displayMode = TrailDisplayMode::Hostile;
    if (static_cast<int>(trails.teleportMode) < 0 || static_cast<int>(trails.teleportMode) > 1) trails.teleportMode = TrailTeleportMode::Tactical;
    if (static_cast<int>(m_settings.playerESP.gearDisplayMode) < 0 || static_cast<int>(m_settings.playerESP.gearDisplayMode) > 2) m_settings.playerESP.gearDisplayMode = GearDisplayMode::Compact;
    if (static_cast<int>(m_settings.playerESP.energyDisplayType) < 0 || static_cast<int>(m_settings.playerESP.energyDisplayType) > 1) m_settings.playerESP.energyDisplayType = EnergyDisplayType::Energy;
}

} // namespace kx
