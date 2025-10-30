#include "ESPTrailRenderer.h"
#include "../Data/ESPData.h"
#include "../Data/EntityRenderContext.h"
#include "../Combat/CombatStateManager.h"
#include "../Combat/CombatState.h"
#include "../Utils/ESPMath.h"
#include "ESPShapeRenderer.h"
#include "../../Core/AppState.h"
#include "../../Game/GameEnums.h"
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/spline.hpp>
#include <glm/geometric.hpp>

namespace kx {

namespace {
    constexpr int SPLINE_SEGMENTS_PER_CURVE = 4;
}

void ESPTrailRenderer::RenderPlayerTrail(
    const FrameContext& context,
    const EntityRenderContext& entityContext,
    const VisualProperties& props)
{
    const auto& settings = AppState::Get().GetSettings();
    const auto& trailSettings = settings.playerESP.trails;
    
    if (!trailSettings.enabled) {
        return;
    }
    
    if (trailSettings.displayMode == TrailDisplayMode::Hostile) {
        if (entityContext.attitude != Game::Attitude::Hostile) {
            return;
        }
    }
    
    const uint64_t now = context.now;
    std::vector<glm::vec3> worldPoints = CollectTrailPoints(context, entityContext, now);
    
    if (worldPoints.size() < 2) {
        return;
    }
    
    std::vector<glm::vec3> smoothedWorldPoints = GenerateSmoothTrail(worldPoints, trailSettings.teleportThreshold);
    
    if (smoothedWorldPoints.size() < 2) {
        return;
    }
    
    ProjectAndRenderTrail(context, smoothedWorldPoints, trailSettings.thickness, 
                         props.fadedEntityColor, props.finalAlpha, settings.appearance.globalOpacity);
}

std::vector<glm::vec3> ESPTrailRenderer::CollectTrailPoints(
    const FrameContext& context,
    const EntityRenderContext& entityContext,
    uint64_t now)
{
    const EntityCombatState* state = context.stateManager.GetState(entityContext.entity->address);
    if (!state || state->positionHistory.empty()) {
        return {};
    }

    const auto& settings = AppState::Get().GetSettings();
    const uint64_t cutoffTime = now - static_cast<uint64_t>(settings.playerESP.trails.maxDuration * 1000.0f);
    
    std::vector<glm::vec3> worldPoints;
    worldPoints.reserve(state->positionHistory.size() + 1);

    for (const auto& historyPoint : state->positionHistory) {
        if (historyPoint.timestamp >= cutoffTime) {
            worldPoints.push_back(historyPoint.position);
        }
    }

    if (worldPoints.empty()) {
        return {};
    }

    glm::vec3 interpolatedHeadPos = entityContext.entity->position;
    if (state->positionHistory.size() >= 2) {
        const auto& P0 = state->positionHistory.back();
        const auto& P1 = state->positionHistory[state->positionHistory.size() - 2];
        
        if (now >= P0.timestamp && P0.timestamp > P1.timestamp) {
            uint64_t timeDiff = P0.timestamp - P1.timestamp;
            if (timeDiff > 0) {
                uint64_t timeSinceP0 = now - P0.timestamp;
                float t = static_cast<float>(timeSinceP0) / static_cast<float>(timeDiff);
                t = glm::clamp(t, 0.0f, 1.0f);
                interpolatedHeadPos = glm::mix(P0.position, entityContext.entity->position, t);
            }
        }
    }
    worldPoints.push_back(interpolatedHeadPos);

    return worldPoints;
}

std::vector<glm::vec3> ESPTrailRenderer::GenerateSmoothTrail(
    const std::vector<glm::vec3>& worldPoints,
    float teleportThreshold)
{
    if (worldPoints.size() < 4) {
        return worldPoints;
    }

    std::vector<glm::vec3> smoothedWorldPoints;
    
    for (size_t i = 0; i < worldPoints.size() - 3; ++i) {
        float maxDist = 0.0f;
        for (size_t j = i; j < i + 3; ++j) {
            float dist = glm::distance(worldPoints[j], worldPoints[j + 1]);
            maxDist = (std::max)(maxDist, dist);
        }
        
        if (maxDist > teleportThreshold) {
            smoothedWorldPoints.push_back(worldPoints[i + 1]);
            continue;
        }
        
        const glm::vec3& p0 = worldPoints[i];
        const glm::vec3& p1 = worldPoints[i + 1];
        const glm::vec3& p2 = worldPoints[i + 2];
        const glm::vec3& p3 = worldPoints[i + 3];
        
        for (int j = 0; j < SPLINE_SEGMENTS_PER_CURVE; ++j) {
            float t = static_cast<float>(j) / static_cast<float>(SPLINE_SEGMENTS_PER_CURVE);
            glm::vec3 smoothedPoint = glm::catmullRom(p0, p1, p2, p3, t);
            smoothedWorldPoints.push_back(smoothedPoint);
        }
    }
    
    smoothedWorldPoints.push_back(worldPoints[worldPoints.size() - 2]);
    smoothedWorldPoints.push_back(worldPoints[worldPoints.size() - 1]);

    return smoothedWorldPoints;
}

void ESPTrailRenderer::ProjectAndRenderTrail(
    const FrameContext& context,
    const std::vector<glm::vec3>& smoothedWorldPoints,
    float thickness,
    ImU32 baseColor,
    float finalAlpha,
    float globalOpacity)
{
    std::vector<ImVec2> screenPoints;
    screenPoints.reserve(smoothedWorldPoints.size());
    
    for (const auto& worldPoint : smoothedWorldPoints) {
        glm::vec2 screenPos;
        if (ESPMath::WorldToScreen(worldPoint, context.camera, context.screenWidth, context.screenHeight, screenPos)) {
            screenPoints.push_back(ImVec2(screenPos.x, screenPos.y));
        }
    }

    if (screenPoints.size() < 2) {
        return;
    }

    for (size_t i = 0; i < screenPoints.size() - 1; ++i) {
        float trailFade = static_cast<float>(i) / static_cast<float>(screenPoints.size() - 1);
        float combinedAlpha = trailFade * finalAlpha * globalOpacity;
        ImU32 fadedColor = ESPShapeRenderer::ApplyAlphaToColor(baseColor, combinedAlpha);
        
        context.drawList->AddLine(screenPoints[i], screenPoints[i + 1], fadedColor, thickness);
    }
}

} // namespace kx

