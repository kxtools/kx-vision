#include "TrailRenderer.h"
#include "../../../Game/Data/FrameData.h"
#include "../../../Game/Data/EntityData.h"
#include "../../../Core/Services/Combat/CombatStateManager.h"
#include "../../../Core/Services/Combat/CombatState.h"
#include "../../../Rendering/Shared/MathUtils.h"
#include "ShapeRenderer.h"
#include "../../../Core/AppState.h"
#include "../../../Game/GameEnums.h"
#include "../Settings/VisualsSettings.h"
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/spline.hpp>
#include <glm/geometric.hpp>

namespace kx {

namespace {
    constexpr int SPLINE_SEGMENTS_PER_CURVE = 4;
    constexpr float TELEPORT_THRESHOLD_METERS = 10.0f;
}

void TrailRenderer::RenderPlayerTrail(
    const FrameContext& context,
    const PlayerEntity& player,
    Game::Attitude attitude,
    const VisualProperties& props,
    const VisualsConfiguration& visualsConfig)
{
    const auto& trailSettings = visualsConfig.playerESP.trails;
    
    if (!trailSettings.enabled) {
        return;
    }
    
    if (trailSettings.displayMode == TrailDisplayMode::Hostile) {
        if (attitude != Game::Attitude::Hostile) {
            return;
        }
    }
    
    const uint64_t now = context.now;
    const EntityCombatState* state = context.stateManager.GetState(player.GetCombatKey());
    if (!state || state->historySize < 2) {
        return;
    }
    
    ImDrawList* drawList = context.drawList;
    const float thickness = trailSettings.thickness;
    const float maxDuration = trailSettings.maxDuration;
    ImU32 baseColor = props.style.fadedEntityColor;
    float finalAlpha = props.style.finalAlpha;
    float globalOpacity = context.settings.appearance.globalOpacity;
    bool renderTeleportConnections = (trailSettings.teleportMode == TrailTeleportMode::Analysis);
    
    constexpr float DASH_LENGTH = 10.0f;
    constexpr float GAP_LENGTH = 5.0f;
    constexpr float TELEPORT_ALPHA = 0.8f;
    
    auto DrawTeleportConnection = [&](const PositionHistoryPoint& startWorld, const PositionHistoryPoint& endWorld) {
        float age = static_cast<float>(now - startWorld.timestamp) / 1000.0f;
        float timeBasedFade = 1.0f - glm::clamp(age / maxDuration, 0.0f, 1.0f);
        timeBasedFade *= timeBasedFade;
        
        ImU32 teleportColor = ShapeRenderer::ApplyAlphaToColor(baseColor, timeBasedFade * TELEPORT_ALPHA * finalAlpha * globalOpacity);
        
        glm::vec2 startScreen, endScreen;
        if (MathUtils::WorldToScreen(startWorld.position, context.camera, context.screenWidth, context.screenHeight, startScreen) &&
            MathUtils::WorldToScreen(endWorld.position, context.camera, context.screenWidth, context.screenHeight, endScreen)) {
            
            ImVec2 start(startScreen.x, startScreen.y);
            ImVec2 end(endScreen.x, endScreen.y);
            
            ImVec2 delta = ImVec2(end.x - start.x, end.y - start.y);
            float lineLength = sqrtf(delta.x * delta.x + delta.y * delta.y);
            
            if (lineLength < 0.01f) return;
            
            ImVec2 direction = ImVec2(delta.x / lineLength, delta.y / lineLength);
            
            for (float i = 0; i < lineLength; i += DASH_LENGTH + GAP_LENGTH) {
                ImVec2 p1 = ImVec2(start.x + direction.x * i, start.y + direction.y * i);
                float dashEnd = (std::min)(i + DASH_LENGTH, lineLength);
                ImVec2 p2 = ImVec2(start.x + direction.x * dashEnd, start.y + direction.y * dashEnd);
                drawList->AddLine(p1, p2, teleportColor, thickness);
            }
        }
    };
    
    auto DrawSplineSegment = [&](const PositionHistoryPoint& p0, const PositionHistoryPoint& p1, 
                                 const PositionHistoryPoint& p2, const PositionHistoryPoint& p3) {
        glm::vec2 screenPrev;
        bool prevValid = false;
        
        if (MathUtils::WorldToScreen(p1.position, context.camera, context.screenWidth, context.screenHeight, screenPrev)) {
            prevValid = true;
        }
        
        for (int i = 1; i <= SPLINE_SEGMENTS_PER_CURVE; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(SPLINE_SEGMENTS_PER_CURVE);
            
            glm::vec3 worldPos = glm::catmullRom(p0.position, p1.position, p2.position, p3.position, t);
            
            uint64_t timeDiff = p2.timestamp - p1.timestamp;
            uint64_t interpTime = p1.timestamp + static_cast<uint64_t>(static_cast<float>(timeDiff) * t);
            
            float age = static_cast<float>(now - interpTime) / 1000.0f;
            float timeFade = 1.0f - glm::clamp(age / maxDuration, 0.0f, 1.0f);
            timeFade *= timeFade;
            
            if (timeFade <= 0.01f) continue;
            
            ImU32 col = ShapeRenderer::ApplyAlphaToColor(baseColor, finalAlpha * timeFade * globalOpacity);
            
            glm::vec2 screenCurr;
            if (MathUtils::WorldToScreen(worldPos, context.camera, context.screenWidth, context.screenHeight, screenCurr)) {
                if (prevValid) {
                    drawList->AddLine(ImVec2(screenPrev.x, screenPrev.y), ImVec2(screenCurr.x, screenCurr.y), col, thickness);
                }
                screenPrev = screenCurr;
                prevValid = true;
            } else {
                prevValid = false;
            }
        }
    };
    
    for (size_t i = 0; i < state->historySize - 1; ++i) {
        const auto& p1 = state->GetHistoryItem(i);
        const auto& p2 = state->GetHistoryItem(i + 1);
        
        float dist = glm::distance(p1.position, p2.position);
        
        if (dist > TELEPORT_THRESHOLD_METERS) {
            if (renderTeleportConnections) {
                DrawTeleportConnection(p1, p2);
            }
            continue;
        }
        
        const auto& p0 = (i > 0) ? state->GetHistoryItem(i - 1) : p1;
        const auto& p3 = (i + 2 < state->historySize) ? state->GetHistoryItem(i + 2) : p2;
        
        DrawSplineSegment(p0, p1, p2, p3);
    }
    
    if (state->historySize > 0) {
        const auto& newestPoint = state->GetHistoryItem(state->historySize - 1);
        if ((now - newestPoint.timestamp) < 150) {
            PositionHistoryPoint interpolatedHeadPoint;
            interpolatedHeadPoint.position = player.position;
            interpolatedHeadPoint.timestamp = now;
            
            if (state->historySize >= 2) {
                const auto& secondNewest = state->GetHistoryItem(state->historySize - 2);
                
                if (now >= newestPoint.timestamp && newestPoint.timestamp > secondNewest.timestamp) {
                    uint64_t timeDiff = newestPoint.timestamp - secondNewest.timestamp;
                    if (timeDiff > 0) {
                        uint64_t timeSinceNewest = now - newestPoint.timestamp;
                        float t = static_cast<float>(timeSinceNewest) / static_cast<float>(timeDiff);
                        t = glm::clamp(t, 0.0f, 1.0f);
                        
                        interpolatedHeadPoint.position = glm::mix(newestPoint.position, player.position, t);
                        
                        uint64_t headTimeRange = now - newestPoint.timestamp;
                        interpolatedHeadPoint.timestamp = newestPoint.timestamp + static_cast<uint64_t>(static_cast<float>(headTimeRange) * t);
                    }
                }
            }
            
            const auto& p0 = (state->historySize >= 2) ? state->GetHistoryItem(state->historySize - 2) : newestPoint;
            const auto& p1 = newestPoint;
            const auto& p2 = interpolatedHeadPoint;
            const auto& p3 = interpolatedHeadPoint;
            DrawSplineSegment(p0, p1, p2, p3);
        }
    }
}

} // namespace kx

