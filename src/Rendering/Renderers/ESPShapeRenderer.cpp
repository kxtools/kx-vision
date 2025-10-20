#define NOMINMAX
#include "ESPShapeRenderer.h"

#include <algorithm>
#include <vector>

#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"
#include "../Utils/ESPMath.h"
#include "../Data/EntityRenderContext.h"

namespace kx {

void ESPShapeRenderer::RenderGadgetSphere(ImDrawList* drawList, const EntityRenderContext& entityContext, Camera& camera,
    const glm::vec2& screenPos, float finalAlpha, unsigned int fadedEntityColor, float scale, float screenWidth, float screenHeight) {
    // --- Final 3D Gyroscope with a Robust LOD to a 2D Circle ---

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
    float circleAlpha = 0.0f;

    if (entityContext.gameplayDistance > GadgetSphere::LOD_TRANSITION_START) {
        // We are in or past the transition zone.
        float range = GadgetSphere::LOD_TRANSITION_END - GadgetSphere::LOD_TRANSITION_START;
        float progress = std::clamp((entityContext.gameplayDistance - GadgetSphere::LOD_TRANSITION_START) / range, 0.0f, 1.0f);

        gyroscopeAlpha = 1.0f - progress; // Fades out from 1.0 to 0.0
        circleAlpha = progress;          // Fades in from 0.0 to 1.0
    }

    // --- RENDER THE 3D GYROSCOPE (if it's visible) ---
    if (gyroscopeAlpha > 0.0f) {
        const float finalLineThickness = std::clamp(GadgetSphere::BASE_THICKNESS * scale, GadgetSphere::MIN_THICKNESS, GadgetSphere::MAX_THICKNESS);

        // --- Project points with camera-facing information ---
        std::vector<ImVec2> screenRingXY, screenRingXZ, screenRingYZ;
        std::vector<float> facingRingXY, facingRingXZ, facingRingYZ;
        bool projection_ok = true;
        
        // Get camera position once for all rings (optimization)
        glm::vec3 cameraPos = camera.GetCameraPosition();
        
        auto project_ring_with_facing = [&](const std::vector<glm::vec3>& local_points, std::vector<ImVec2>& screen_points, std::vector<float>& facing_points) {
            if (!projection_ok) return;
            screen_points.reserve(local_points.size());
            facing_points.reserve(local_points.size());
            
            for (const auto& point : local_points) {
                glm::vec3 worldPoint = entityContext.position + point;
                glm::vec2 sp;
                
                if (ESPMath::WorldToScreen(worldPoint, camera, screenWidth, screenHeight, sp)) {
                    screen_points.push_back(ImVec2(sp.x, sp.y));
                    
                    // Calculate camera-facing factor using dot product
                    glm::vec3 viewDir = glm::normalize(worldPoint - cameraPos);
                    glm::vec3 outwardNormal = glm::normalize(point); // point is local offset from sphere center
                    float facingFactor = glm::dot(outwardNormal, -viewDir);
                    
                    facing_points.push_back(facingFactor);
                }
                else { 
                    projection_ok = false; 
                    screen_points.clear(); 
                    facing_points.clear(); 
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

void ESPShapeRenderer::RenderGadgetCircle(ImDrawList* drawList, const glm::vec2& screenPos, float radius, unsigned int color, float thickness) {
    drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), radius, color, 0, thickness);
}

void ESPShapeRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                        unsigned int color, float thickness) {
    // Main box only - no corner indicators for cleaner appearance
    drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, thickness);
}

void ESPShapeRenderer::RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos,
                                       unsigned int color, float radius) {
    // Extract fade alpha from the color parameter
    float fadeAlpha = ((color >> 24) & 0xFF) / 255.0f;

    // Small, minimalistic dot with subtle outline for visibility
    // Dark outline with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_SHADOW_ALPHA * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), radius, IM_COL32(0, 0, 0, shadowAlpha));
    // Main dot using entity color (already has faded alpha)
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), radius * RenderingLayout::DOT_RADIUS_MULTIPLIER, color);
}

void ESPShapeRenderer::RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos,
                                            float fadeAlpha, float radius) {
    ImVec2 pos(feetPos.x, feetPos.y);

    // Shadow with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(RenderingLayout::PLAYER_NAME_BORDER_ALPHA * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(pos.x + RenderingLayout::TEXT_SHADOW_OFFSET, pos.y + RenderingLayout::TEXT_SHADOW_OFFSET), radius, IM_COL32(0, 0, 0, shadowAlpha));

    // Dot with distance fade
    unsigned int dotAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    drawList->AddCircleFilled(pos, radius * RenderingLayout::DOT_RADIUS_MULTIPLIER, IM_COL32(255, 255, 255, dotAlpha));
}

unsigned int ESPShapeRenderer::ApplyAlphaToColor(unsigned int color, float alpha) {
    // Extract RGBA components
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    int originalAlpha = (color >> 24) & 0xFF;
    
    // Apply alpha multiplier while preserving original alpha intentions
    int newAlpha = static_cast<int>(originalAlpha * alpha);
    newAlpha = (newAlpha < 0) ? 0 : (newAlpha > 255) ? 255 : newAlpha; // Clamp to valid range
    
    return IM_COL32(r, g, b, newAlpha);
}

} // namespace kx
