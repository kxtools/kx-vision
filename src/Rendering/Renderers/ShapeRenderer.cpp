#define NOMINMAX
#include "ShapeRenderer.h"

#include <algorithm>
#include <array>
#include <vector>

#include "../../../libs/ImGui/imgui.h"
#include "../Shared/MathUtils.h"
#include "../Data/FrameData.h"
#include "Shared/LayoutConstants.h"

namespace kx {

void ShapeRenderer::RenderGyroscopicOverlay(ImDrawList* drawList, 
    const glm::vec3& worldPos, 
    float gameplayDistance,
    Camera& camera,
    float screenWidth, float screenHeight, 
    float finalAlpha, unsigned int fadedEntityColor, float scale, float globalOpacity) 
{
    // --- Define the 3D sphere's geometric properties ---
    const int NUM_RING_POINTS = GadgetSphere::NUM_RING_POINTS;
    const float PI = 3.14159265359f;
    const float VERTICAL_RADIUS = GadgetSphere::VERTICAL_RADIUS;
    const float HORIZONTAL_RADIUS = VERTICAL_RADIUS * GadgetSphere::HORIZONTAL_RADIUS_RATIO;

    // --- Define vertices (precomputed and cached) ---
    static std::vector<glm::vec3> localRingXY, localRingXZ, localRingYZ;
    if (localRingXY.empty()) {
        localRingXY.reserve(NUM_RING_POINTS + 1);
        localRingXZ.reserve(NUM_RING_POINTS + 1);
        localRingYZ.reserve(NUM_RING_POINTS + 1);
        for (int i = 0; i <= NUM_RING_POINTS; ++i) {
            float angle = 2.0f * PI * i / NUM_RING_POINTS;
            float s = sin(angle), c = cos(angle);
            localRingXY.emplace_back(c * HORIZONTAL_RADIUS, s * HORIZONTAL_RADIUS, 0);
            localRingXZ.emplace_back(c * VERTICAL_RADIUS, 0, s * VERTICAL_RADIUS);
            localRingYZ.emplace_back(0, c * VERTICAL_RADIUS, s * VERTICAL_RADIUS);
        }
    }

    // --- 1. LOD (Level of Detail) Calculation ---
    float gyroscopeAlpha = 1.0f;

    // Use the passed 'gameplayDistance' directly
    if (gameplayDistance > GadgetSphere::LOD_TRANSITION_START) {
        float range = GadgetSphere::LOD_TRANSITION_END - GadgetSphere::LOD_TRANSITION_START;
        float progress = std::clamp((gameplayDistance - GadgetSphere::LOD_TRANSITION_START) / range, 0.0f, 1.0f);
        gyroscopeAlpha = 1.0f - progress; 
    }

    if (gyroscopeAlpha > 0.0f) {
        const float finalLineThickness = std::clamp(GadgetSphere::BASE_THICKNESS * scale, GadgetSphere::MIN_THICKNESS, GadgetSphere::MAX_THICKNESS);

        std::vector<ImVec2> screenRingXY, screenRingXZ, screenRingYZ;
        std::vector<float> facingRingXY, facingRingXZ, facingRingYZ;
        bool projection_ok = true;
        glm::vec3 cameraPos = camera.GetCameraPosition();
        
        auto project_ring_with_facing = [&](const std::vector<glm::vec3>& local_points, std::vector<ImVec2>& screen_points, std::vector<float>& facing_points) {
            if (!projection_ok) return;
            screen_points.reserve(local_points.size());
            facing_points.reserve(local_points.size());
            
            for (const auto& point : local_points) {
                // Use the passed 'worldPos' directly
                glm::vec3 currentWorldPoint = worldPos + point;
                glm::vec2 sp;
                
                if (MathUtils::WorldToScreen(currentWorldPoint, camera, screenWidth, screenHeight, sp)) {
                    screen_points.push_back(ImVec2(sp.x, sp.y));
                    
                    glm::vec3 viewDir = glm::normalize(currentWorldPoint - cameraPos);
                    glm::vec3 outwardNormal = glm::normalize(point);
                    float facingFactor = glm::dot(outwardNormal, -viewDir);
                    
                    facing_points.push_back(facingFactor);
                }
                else { 
                    projection_ok = false; 
                    return; 
                }
            }
        };
        
        project_ring_with_facing(localRingXY, screenRingXY, facingRingXY);
        project_ring_with_facing(localRingXZ, screenRingXZ, facingRingXZ);
        project_ring_with_facing(localRingYZ, screenRingYZ, facingRingYZ);

        // --- Draw the 3D sphere with camera-facing rendering ---
        if (projection_ok) {
            // Define the base color for all rings
            unsigned int masterAlpha = (fadedEntityColor >> 24) & 0xFF;
            unsigned int finalLODAlpha = static_cast<unsigned int>(masterAlpha * gyroscopeAlpha);
            ImU32 baseColor = (fadedEntityColor & 0x00FFFFFF) | (finalLODAlpha << 24);

            // Helper function to render ring with camera-facing segments
            auto render_ring_with_facing = [&](const std::vector<ImVec2>& screen_points, const std::vector<float>& facing_points, const char* ring_name) {
                if (screen_points.empty() || facing_points.empty()) return;
                
                // Render each segment individually with facing-based modulation
                for (size_t i = 0; i < screen_points.size(); ++i) {
                    size_t next_i = (i + 1) % screen_points.size();
                    
                    // Calculate average facing factor for this segment
                    float avgFacing = (facing_points[i] + facing_points[next_i]) * 0.5f;
                    
                    // Map facing factor from [-1, 1] to [0, 1]
                    // facing = 1.0 (toward camera) → normalized = 1.0 → bright
                    // facing = -1.0 (away from camera) → normalized = 0.0 → dim
                    float normalizedFacing = (avgFacing + 1.0f) * 0.5f;
                    normalizedFacing = std::clamp(normalizedFacing, 0.0f, 1.0f);
                    
                    // Apply facing-based modulation
                    float brightnessFactor = GadgetSphere::ENABLE_PER_SEGMENT_DEPTH ? 
                        GadgetSphere::DEPTH_BRIGHTNESS_MIN + (GadgetSphere::DEPTH_BRIGHTNESS_MAX - GadgetSphere::DEPTH_BRIGHTNESS_MIN) * normalizedFacing : 1.0f;
                    float thicknessFactor = GadgetSphere::ENABLE_PER_SEGMENT_DEPTH ? 
                        GadgetSphere::DEPTH_THICKNESS_MIN + (GadgetSphere::DEPTH_THICKNESS_MAX - GadgetSphere::DEPTH_THICKNESS_MIN) * normalizedFacing : 1.0f;
                    
                    // Calculate modulated color and thickness
                    ImU32 segmentColor = baseColor;
                    if (GadgetSphere::ENABLE_PER_SEGMENT_DEPTH && brightnessFactor < 1.0f) {
                        // Extract RGB components and apply brightness modulation
                        int r = (baseColor >> IM_COL32_R_SHIFT) & 0xFF;
                        int g = (baseColor >> IM_COL32_G_SHIFT) & 0xFF;
                        int b = (baseColor >> IM_COL32_B_SHIFT) & 0xFF;
                        int a = (baseColor >> IM_COL32_A_SHIFT) & 0xFF;
                        
                        r = static_cast<int>(r * brightnessFactor);
                        g = static_cast<int>(g * brightnessFactor);
                        b = static_cast<int>(b * brightnessFactor);
                        
                        segmentColor = IM_COL32(r, g, b, a);
                    }
                    
                    // Apply global opacity to gadget sphere rings
                    segmentColor = ApplyAlphaToColor(segmentColor, globalOpacity);
                    
                    float segmentThickness = finalLineThickness * thicknessFactor;
                    
                    // Draw the segment
                    drawList->AddLine(screen_points[i], screen_points[next_i], segmentColor, segmentThickness);
                }
            };
            
            // Optional: Ring facing sorting for proper back-to-front rendering
            struct RingData {
                const std::vector<ImVec2>* screenPoints;
                const std::vector<float>* facingPoints;
                const char* name;
                float avgFacing;
            };
            
            std::vector<RingData> rings = {
                {&screenRingXY, &facingRingXY, "XY", 0.0f},
                {&screenRingXZ, &facingRingXZ, "XZ", 0.0f},
                {&screenRingYZ, &facingRingYZ, "YZ", 0.0f}
            };
            
            // Calculate average facing for each ring
            for (auto& ring : rings) {
                if (!ring.facingPoints->empty()) {
                    float sum = 0.0f;
                    for (float facing : *ring.facingPoints) {
                        sum += facing;
                    }
                    ring.avgFacing = sum / ring.facingPoints->size();
                }
            }
            
            // Sort rings by facing (least facing first for proper layering)
            std::sort(rings.begin(), rings.end(), [](const RingData& a, const RingData& b) {
                return a.avgFacing < b.avgFacing; // Lower facing = farther = draw first
            });
            
            // Render rings in sorted order
            for (const auto& ring : rings) {
                render_ring_with_facing(*ring.screenPoints, *ring.facingPoints, ring.name);
            }
        }
    }
}

void ShapeRenderer::RenderGadgetCircle(ImDrawList* drawList, const glm::vec2& screenPos, float radius, unsigned int color, float thickness, float globalOpacity) {
    // Apply global opacity to gadget circles
    unsigned int finalColor = ApplyAlphaToColor(color, globalOpacity);
    drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), radius, finalColor, 0, thickness);
}

void ShapeRenderer::RenderWireframeBox(ImDrawList* drawList, const VisualProperties& props, unsigned int color, float thickness, float globalOpacity) {
    // This array defines the 12 edges of a cube by connecting the indices of the corners.
    // The corner indices match the order defined in Calculate3DBoundingBox.
    const std::array<std::pair<int, int>, 12> edges = {{
        // Bottom face
        {0, 1}, {1, 3}, {3, 2}, {2, 0},
        // Top face
        {4, 5}, {5, 7}, {7, 6}, {6, 4},
        // Vertical connectors
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    }};

    // Apply global opacity from settings
    unsigned int finalColor = ApplyAlphaToColor(color, globalOpacity);

    // Render each edge
    for (const auto& edge : edges) {
        // Only draw an edge if BOTH of its corners were successfully projected.
        // This prevents lines from being drawn from off-screen, creating visual artifacts.
        if (props.cornerValidity[edge.first] && props.cornerValidity[edge.second]) {
            const auto& p1 = props.projectedCorners[edge.first];
            const auto& p2 = props.projectedCorners[edge.second];
            drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), finalColor, thickness);
        }
    }
}

void ShapeRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                        unsigned int color, float thickness, float globalOpacity) {
    // Apply global opacity to bounding boxes
    unsigned int finalColor = ApplyAlphaToColor(color, globalOpacity);
    
    // Dark outer stroke for better visibility (consistent with health bar rendering)
    const float outset = 1.0f; // 1px outside, feels "harder" and more separated
    float mainAlpha = ((finalColor >> 24) & 0xFF) / 255.0f;
    unsigned int strokeAlpha = static_cast<unsigned int>(180 * mainAlpha); // 70% opacity
    unsigned int strokeColor = IM_COL32(0, 0, 0, strokeAlpha);
    
    // Outer stroke (matches health bar pattern: 1px offset, 1px thickness)
    ImVec2 strokeMin(boxMin.x - outset, boxMin.y - outset);
    ImVec2 strokeMax(boxMax.x + outset, boxMax.y + outset);
    drawList->AddRect(strokeMin, strokeMax, strokeColor, 0.0f, 0, 1.0f);
    
    // Main colored box
    drawList->AddRect(boxMin, boxMax, finalColor, 0.0f, 0, thickness);
}

void ShapeRenderer::RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos,
                                       unsigned int color, float radius, float globalOpacity) {
    // Apply global opacity to colored dots
    unsigned int finalColor = ApplyAlphaToColor(color, globalOpacity);
    
    // Extract fade alpha from the final color parameter
    float fadeAlpha = ((finalColor >> 24) & 0xFF) / 255.0f;

    // Small, minimalistic dot with subtle outline for visibility
    // Dark outline with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_SHADOW_ALPHA * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), radius, IM_COL32(0, 0, 0, shadowAlpha));
    // Main dot using entity color (already has faded alpha)
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), radius * RenderingLayout::DOT_RADIUS_MULTIPLIER, finalColor);
}

void ShapeRenderer::RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos,
                                            float fadeAlpha, float radius, float globalOpacity) {
    // Apply global opacity to natural dots
    float combinedAlpha = fadeAlpha * globalOpacity;
    
    ImVec2 pos(feetPos.x, feetPos.y);

    // Shadow with combined alpha
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_BORDER_ALPHA * combinedAlpha);
    drawList->AddCircleFilled(ImVec2(pos.x + RenderingLayout::TEXT_SHADOW_OFFSET, pos.y + RenderingLayout::TEXT_SHADOW_OFFSET), radius, IM_COL32(0, 0, 0, shadowAlpha));

    // Dot with combined alpha
    unsigned int dotAlpha = static_cast<unsigned int>(255 * combinedAlpha);
    drawList->AddCircleFilled(pos, radius * RenderingLayout::DOT_RADIUS_MULTIPLIER, IM_COL32(255, 255, 255, dotAlpha));
}

unsigned int ShapeRenderer::ApplyAlphaToColor(unsigned int color, float alpha) {
    // Extract RGBA components using ImGui's standard bit shifts
    int r = (color >> IM_COL32_R_SHIFT) & 0xFF;
    int g = (color >> IM_COL32_G_SHIFT) & 0xFF;
    int b = (color >> IM_COL32_B_SHIFT) & 0xFF;
    int originalAlpha = (color >> IM_COL32_A_SHIFT) & 0xFF;
    
    // Apply alpha multiplier while preserving original alpha intentions
    int newAlpha = static_cast<int>(originalAlpha * alpha);
    newAlpha = (newAlpha < 0) ? 0 : (newAlpha > 255) ? 255 : newAlpha; // Clamp to valid range
    
    return IM_COL32(r, g, b, newAlpha);
}

} // namespace kx
