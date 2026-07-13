#pragma once

#include "../../Core/Architecture/FeatureWithSettings.h"
#include "Core/MasterRenderer.h"
#include "Logic/AdaptiveFarPlaneCalculator.h"
#include "Settings/VisualsSettings.h"
#include "Combat/VisualCombatTracker.h"
#include <memory>

namespace kx {

class Camera;
class EntityManager;
struct MumbleLinkData;

/**
 * @brief Core visual rendering feature wrapping the MasterRenderer.
 * 
 * This feature handles ESP rendering for players, NPCs, objects, and other entities.
 * It owns the MasterRenderer instance and provides the UI tabs for ESP configuration.
 */
class VisualsFeature : public FeatureWithSettings<VisualsConfiguration> {
public:
    VisualsFeature();
    ~VisualsFeature() override = default;

    bool Initialize(const ServiceContext& ctx) override;
    void Shutdown() override;
    void Update(float deltaTime, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx) override;
    void RenderDrawList(ImDrawList* drawList, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx) override;
    void OnTabRender() override;
    const char* GetName() const override { return "Visuals"; }
    
private:
    void NormalizeSettings() override;
    
    std::unique_ptr<MasterRenderer> m_masterRenderer;
    AdaptiveFarPlaneCalculator m_adaptiveFarPlane;
    VisualCombatTracker m_visualCombatTracker;
    
    // Non-owning pointers to services (populated during Initialize)
    EntityManager* m_entityManager = nullptr;
    Camera* m_camera = nullptr;
};

} // namespace kx
