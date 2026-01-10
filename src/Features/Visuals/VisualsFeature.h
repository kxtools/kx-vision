#pragma once

#include "../../Core/Architecture/IFeature.h"
#include "Core/MasterRenderer.h"
#include "Settings/VisualsSettings.h"
#include <memory>

namespace kx {

class Camera;
struct MumbleLinkData;

/**
 * @brief Core visual rendering feature wrapping the MasterRenderer.
 * 
 * This feature handles ESP rendering for players, NPCs, objects, and other entities.
 * It owns the MasterRenderer instance and provides the UI tabs for ESP configuration.
 */
class VisualsFeature : public IFeature {
public:
    VisualsFeature();
    ~VisualsFeature() override = default;

    bool Initialize() override;
    void Update(float deltaTime, const FrameGameData& frameData) override;
    void RenderDrawList(ImDrawList* drawList) override;
    void OnMenuRender() override;
    const char* GetName() const override { return "Visuals"; }
    
    void LoadSettings(const nlohmann::json& j) override;
    void SaveSettings(nlohmann::json& j) override;

private:
    static constexpr const char* SettingsKey = "visuals";
    
    std::unique_ptr<MasterRenderer> m_masterRenderer;
    VisualsConfiguration m_settings;
};

} // namespace kx
