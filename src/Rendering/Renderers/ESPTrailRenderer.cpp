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
    constexpr float TELEPORT_THRESHOLD_METERS = 10.0f;
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
    
    TrailSegmentData segmentData = GenerateSmoothTrail(worldPoints, TELEPORT_THRESHOLD_METERS);
    
    if (segmentData.segments.empty()) {
        return;
    }
    
    bool renderTeleportConnections = (trailSettings.teleportMode == TrailTeleportMode::Analysis);
    
    ProjectAndRenderTrail(context, segmentData, trailSettings.thickness, 
                         props.fadedEntityColor, props.finalAlpha, settings.appearance.globalOpacity,
                         renderTeleportConnections);
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

TrailSegmentData ESPTrailRenderer::GenerateSmoothTrail(
    const std::vector<glm::vec3>& worldPoints,
    float teleportThreshold)
{
    TrailSegmentData result;
    
    if (worldPoints.size() < 2) {
        return result;
    }

    std::vector<glm::vec3> currentSegment;
    currentSegment.push_back(worldPoints[0]);

    for (size_t i = 1; i < worldPoints.size(); ++i) {
        float dist = glm::distance(worldPoints[i - 1], worldPoints[i]);
        
        if (dist > teleportThreshold) {
            if (currentSegment.size() >= 2) {
                result.segments.push_back(currentSegment);
            }
            result.teleportConnections.push_back({worldPoints[i - 1], worldPoints[i]});
            currentSegment.clear();
            currentSegment.push_back(worldPoints[i]);
        } else {
            currentSegment.push_back(worldPoints[i]);
        }
    }

    if (currentSegment.size() >= 2) {
        result.segments.push_back(currentSegment);
    }

    std::vector<std::vector<glm::vec3>> smoothedSegments;
    
    for (const auto& segment : result.segments) {
        if (segment.size() < 4) {
            smoothedSegments.push_back(segment);
            continue;
        }

        std::vector<glm::vec3> smoothedSegment;
        
        for (size_t i = 0; i < segment.size() - 3; ++i) {
            const glm::vec3& p0 = segment[i];
            const glm::vec3& p1 = segment[i + 1];
            const glm::vec3& p2 = segment[i + 2];
            const glm::vec3& p3 = segment[i + 3];
            
            for (int j = 0; j < SPLINE_SEGMENTS_PER_CURVE; ++j) {
                float t = static_cast<float>(j) / static_cast<float>(SPLINE_SEGMENTS_PER_CURVE);
                glm::vec3 smoothedPoint = glm::catmullRom(p0, p1, p2, p3, t);
                smoothedSegment.push_back(smoothedPoint);
            }
        }
        
        smoothedSegment.push_back(segment[segment.size() - 2]);
        smoothedSegment.push_back(segment[segment.size() - 1]);

        smoothedSegments.push_back(smoothedSegment);
    }

    result.segments = smoothedSegments;
    return result;
}

void ESPTrailRenderer::ProjectAndRenderTrail(
    const FrameContext& context,
    const TrailSegmentData& segmentData,
    float thickness,
    ImU32 baseColor,
    float finalAlpha,
    float globalOpacity,
    bool renderTeleportConnections)
{
    for (const auto& segment : segmentData.segments) {
        if (segment.size() < 2) {
            continue;
        }

        std::vector<ImVec2> screenPoints;
        screenPoints.reserve(segment.size());
        
        for (const auto& worldPoint : segment) {
            glm::vec2 screenPos;
            if (ESPMath::WorldToScreen(worldPoint, context.camera, context.screenWidth, context.screenHeight, screenPos)) {
                screenPoints.push_back(ImVec2(screenPos.x, screenPos.y));
            }
        }

        if (screenPoints.size() < 2) {
            continue;
        }

        for (size_t i = 0; i < screenPoints.size() - 1; ++i) {
            float trailFade = static_cast<float>(i) / static_cast<float>(screenPoints.size() - 1);
            float combinedAlpha = trailFade * finalAlpha * globalOpacity;
            ImU32 fadedColor = ESPShapeRenderer::ApplyAlphaToColor(baseColor, combinedAlpha);
            
            context.drawList->AddLine(screenPoints[i], screenPoints[i + 1], fadedColor, thickness);
        }
    }

    if (renderTeleportConnections) {
        constexpr float DASH_LENGTH = 10.0f;
        constexpr float GAP_LENGTH = 5.0f;
        constexpr float TELEPORT_ALPHA = 0.8f;
        
        ImU32 teleportColor = ESPShapeRenderer::ApplyAlphaToColor(baseColor, TELEPORT_ALPHA * finalAlpha * globalOpacity);
        
        for (const auto& [startWorld, endWorld] : segmentData.teleportConnections) {
            glm::vec2 startScreen, endScreen;
            if (ESPMath::WorldToScreen(startWorld, context.camera, context.screenWidth, context.screenHeight, startScreen) &&
                ESPMath::WorldToScreen(endWorld, context.camera, context.screenWidth, context.screenHeight, endScreen)) {
                
                ImVec2 start(startScreen.x, startScreen.y);
                ImVec2 end(endScreen.x, endScreen.y);
                
                ImVec2 delta = ImVec2(end.x - start.x, end.y - start.y);
                float lineLength = sqrtf(delta.x * delta.x + delta.y * delta.y);
                
                if (lineLength < 0.01f) continue;
                
                ImVec2 direction = ImVec2(delta.x / lineLength, delta.y / lineLength);
                
                for (float i = 0; i < lineLength; i += DASH_LENGTH + GAP_LENGTH) {
                    ImVec2 p1 = ImVec2(start.x + direction.x * i, start.y + direction.y * i);
                    float dashEnd = (std::min)(i + DASH_LENGTH, lineLength);
                    ImVec2 p2 = ImVec2(start.x + direction.x * dashEnd, start.y + direction.y * dashEnd);
                    context.drawList->AddLine(p1, p2, teleportColor, 2.0f);
                }
            }
        }
    }
}

} // namespace kx

