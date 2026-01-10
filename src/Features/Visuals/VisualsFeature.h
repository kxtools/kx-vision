#pragma once

#include "../../Core/Architecture/IFeature.h"
#include "Core/MasterRenderer.h"
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
    void Update(float deltaTime) override;
    void RenderDrawList(ImDrawList* drawList) override;
    void OnMenuRender() override;
    const char* GetName() const override { return "Visuals"; }

private:
    std::unique_ptr<MasterRenderer> m_masterRenderer;
};

} // namespace kx
