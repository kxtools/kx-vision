#define NOMINMAX

#include "ScreenProjector.h"
#include "../../Core/AppState.h"
#include "../Data/RenderableData.h"
#include "../Shared/LayoutConstants.h"
#include "../Shared/MathUtils.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>
#include <cmath>
#include <array>
#include <cfloat>

namespace kx::Renderers {

bool ScreenProjector::Project(
    const RenderableEntity& entity,
    const Camera& camera,
    float screenW,
    float screenH,
    const VisualStyle& style,
    ScreenGeometry& outGeometry) {
    
    // Calculate world bounds - prefer physics dimensions if available
    float worldWidth, worldDepth, worldHeight;
    if (entity.hasPhysicsDimensions) {
        worldWidth = entity.physicsWidth;
        worldDepth = entity.physicsDepth;
        worldHeight = entity.physicsHeight;
    } else {
        GetWorldBoundsForEntity(entity.entityType, worldWidth, worldDepth, worldHeight);
    }
    
    // OPTIMIZED: Calculate clip position ONCE
    // Get matrices by reference (fast)
    const glm::mat4& view = camera.GetViewMatrix();
    const glm::mat4& proj = camera.GetProjectionMatrix();
    
    // Perform the heavy math one time
    glm::vec4 clipPos = proj * view * glm::vec4(entity.position, 1.0f);
    
    // Early culling: Check if entity is far behind camera before expensive 8-corner projection
    // w < -2.0f allows objects slightly behind camera to still process (prevents popping)
    if (clipPos.w < -2.0f) {
        // Far behind camera, safe to cull even large objects
        return false;
    }
    
    // Calculate screen position (Reuse clipPos)
    // This replaces the call to MathUtils::ProjectToScreen
    bool isOriginValid = false;
    if (clipPos.w > 0.0f) {
        // Perspective Division
        glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
        
        // Viewport Transform (matches MathUtils logic)
        outGeometry.screenPos.x = (ndc.x + 1.0f) * 0.5f * screenW;
        outGeometry.screenPos.y = (1.0f - ndc.y) * 0.5f * screenH; // 1.0 - y because screen Y is down
        isOriginValid = true;
    } else {
        // Behind camera (but passed the -2.0f check)
        outGeometry.screenPos = glm::vec2(0.0f);
        isOriginValid = false;
    }
    
    // Project based on entity type
    if (entity.entityType == EntityTypes::Gadget || entity.entityType == EntityTypes::AttackTarget) {
        ProjectGadget(entity, camera, screenW, screenH, outGeometry, style.scale, isOriginValid);
    } else {
        ProjectCharacter(entity, camera, screenW, screenH, outGeometry, style.scale, isOriginValid);
    }
    
    // Perform frustum culling check
    bool overlapsX = outGeometry.boxMin.x < screenW && outGeometry.boxMax.x > 0;
    bool overlapsY = outGeometry.boxMin.y < screenH && outGeometry.boxMax.y > 0;
    outGeometry.isOnScreen = overlapsX && overlapsY;
    
    return outGeometry.isOnScreen;
}

void ScreenProjector::Calculate3DBoundingBox(
    const glm::vec3& entityPos,
    float worldWidth,
    float worldDepth,
    float worldHeight,
    const Camera& camera,
    float screenWidth,
    float screenHeight,
    ScreenGeometry& geometry,
    bool& outValid) {
    
    const std::array<glm::vec3, 8> worldCorners = {
        entityPos + glm::vec3(-worldWidth/2, 0.0f, -worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, 0.0f, -worldDepth/2),
        entityPos + glm::vec3(-worldWidth/2, 0.0f,  worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, 0.0f,  worldDepth/2),
        entityPos + glm::vec3(-worldWidth/2, worldHeight, -worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, worldHeight, -worldDepth/2),
        entityPos + glm::vec3(-worldWidth/2, worldHeight,  worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, worldHeight,  worldDepth/2)
    };
    
    float minX = FLT_MAX, minY = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX;
    int validCornerCount = 0;
    
    for (int i = 0; i < 8; ++i) {
        if (MathUtils::ProjectToScreen(worldCorners[i], camera, screenWidth, screenHeight, geometry.projectedCorners[i])) {
            geometry.cornerValidity[i] = true;
            validCornerCount++;
            
            minX = std::min(minX, geometry.projectedCorners[i].x);
            minY = std::min(minY, geometry.projectedCorners[i].y);
            maxX = std::max(maxX, geometry.projectedCorners[i].x);
            maxY = std::max(maxY, geometry.projectedCorners[i].y);
        } else {
            geometry.cornerValidity[i] = false;
        }
    }
    
    outValid = (validCornerCount > 0);
    
    if (outValid) {
        geometry.boxMin = ImVec2(minX, minY);
        geometry.boxMax = ImVec2(maxX, maxY);
    }
}

void ScreenProjector::GetWorldBoundsForEntity(
    EntityTypes entityType,
    float& outWidth,
    float& outDepth,
    float& outHeight) {
    switch (entityType) {
        case EntityTypes::Player:
            outWidth = EntityWorldBounds::PLAYER_WORLD_WIDTH;
            outDepth = EntityWorldBounds::PLAYER_WORLD_DEPTH;
            outHeight = EntityWorldBounds::PLAYER_WORLD_HEIGHT;
            break;
        
        case EntityTypes::AttackTarget:
        case EntityTypes::Gadget:
            outWidth = EntityWorldBounds::GADGET_WORLD_WIDTH;
            outDepth = EntityWorldBounds::GADGET_WORLD_DEPTH;
            outHeight = EntityWorldBounds::GADGET_WORLD_HEIGHT;
            break;
        
        case EntityTypes::NPC:
        default:
            outWidth = EntityWorldBounds::NPC_WORLD_WIDTH;
            outDepth = EntityWorldBounds::NPC_WORLD_DEPTH;
            outHeight = EntityWorldBounds::NPC_WORLD_HEIGHT;
            break;
    }
}

void ScreenProjector::CalculateEntityBoxDimensions(EntityTypes entityType, float scale,
                                                    float& outBoxWidth, float& outBoxHeight) {
    const auto& settings = AppState::Get().GetSettings();
    
    switch (entityType) {
    case EntityTypes::Player:
        outBoxHeight = settings.sizes.baseBoxHeight * scale;
        outBoxWidth = settings.sizes.baseBoxWidth * scale;
        if (outBoxHeight < MinimumSizes::PLAYER_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::PLAYER_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::PLAYER_MIN_WIDTH;
        }
        break;
        
    case EntityTypes::NPC:
        outBoxHeight = settings.sizes.baseBoxWidth * scale;
        outBoxWidth = settings.sizes.baseBoxWidth * scale;
        if (outBoxHeight < MinimumSizes::NPC_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::NPC_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::NPC_MIN_WIDTH;
        }
        break;
        
    case EntityTypes::Gadget:
    case EntityTypes::AttackTarget:
        outBoxHeight = settings.sizes.baseBoxHeight * scale;
        outBoxWidth = settings.sizes.baseBoxWidth * scale;
        break;
        
    default:
        outBoxHeight = settings.sizes.baseBoxHeight * scale;
        outBoxWidth = settings.sizes.baseBoxWidth * scale;
        if (outBoxHeight < MinimumSizes::PLAYER_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::PLAYER_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::PLAYER_MIN_WIDTH;
        }
        break;
    }
}

void ScreenProjector::ApplyFallback2DBox(
    const RenderableEntity& entity,
    ScreenGeometry& geometry,
    float scale,
    const glm::vec2& screenPos) {
    float boxWidth, boxHeight;
    CalculateEntityBoxDimensions(entity.entityType, scale, boxWidth, boxHeight);
    geometry.boxMin = ImVec2(screenPos.x - boxWidth / 2, screenPos.y - boxHeight);
    geometry.boxMax = ImVec2(screenPos.x + boxWidth / 2, screenPos.y);
}

void ScreenProjector::ProjectGadget(
    const RenderableEntity& entity,
    const Camera& camera,
    float screenWidth,
    float screenHeight,
    ScreenGeometry& geometry,
    float scale,
    bool isOriginValid) {
    const auto& settings = AppState::Get().GetSettings();
    
    float baseRadius = settings.sizes.baseBoxWidth * EntitySizeRatios::GADGET_CIRCLE_RADIUS_RATIO;
    geometry.circleRadius = (std::max)(MinimumSizes::GADGET_MIN_WIDTH / 2.0f, baseRadius * scale);

    geometry.center = ImVec2(geometry.screenPos.x, geometry.screenPos.y);
    
    float worldWidth, worldDepth, worldHeight;
    if (entity.hasPhysicsDimensions) {
        worldWidth = entity.physicsWidth;
        worldDepth = entity.physicsDepth;
        worldHeight = entity.physicsHeight;
    } else {
        GetWorldBoundsForEntity(entity.entityType, worldWidth, worldDepth, worldHeight);
    }
    
    bool boxValid = false;
    Calculate3DBoundingBox(
        entity.position,
        worldWidth,
        worldDepth,
        worldHeight,
        camera,
        screenWidth,
        screenHeight,
        geometry,
        boxValid
    );
    
    if (!boxValid) {
        if (isOriginValid) {
            geometry.boxMin = ImVec2(geometry.screenPos.x - geometry.circleRadius, geometry.screenPos.y - geometry.circleRadius);
            geometry.boxMax = ImVec2(geometry.screenPos.x + geometry.circleRadius, geometry.screenPos.y + geometry.circleRadius);
        }
    }
}

void ScreenProjector::ProjectCharacter(
    const RenderableEntity& entity,
    const Camera& camera,
    float screenWidth,
    float screenHeight,
    ScreenGeometry& geometry,
    float scale,
    bool isOriginValid) {
    float worldWidth, worldDepth, worldHeight;
    
    if (entity.hasPhysicsDimensions) {
        worldWidth = entity.physicsWidth;
        worldDepth = entity.physicsDepth;
        worldHeight = entity.physicsHeight;
    } else {
        GetWorldBoundsForEntity(entity.entityType, worldWidth, worldDepth, worldHeight);
    }
    
    bool boxValid = false;
    Calculate3DBoundingBox(
        entity.position,
        worldWidth,
        worldDepth,
        worldHeight,
        camera,
        screenWidth,
        screenHeight,
        geometry,
        boxValid
    );
    
    if (!boxValid) {
        if (isOriginValid) {
            ApplyFallback2DBox(entity, geometry, scale, geometry.screenPos);
        }
    }
    
    geometry.center = ImVec2(
        (geometry.boxMin.x + geometry.boxMax.x) / 2,
        (geometry.boxMin.y + geometry.boxMax.y) / 2
    );
    geometry.circleRadius = 0.0f;
}

} // namespace kx::Renderers

