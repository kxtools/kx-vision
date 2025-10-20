#include "ESPShapeRenderer.h"

#include <algorithm>

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

        // --- Project points ---
        std::vector<ImVec2> screenRingXY, screenRingXZ, screenRingYZ;
        bool projection_ok = true;
        auto project_ring = [&](const std::vector<glm::vec3>& local_points, std::vector<ImVec2>& screen_points) {
            if (!projection_ok) return;
            screen_points.reserve(local_points.size());
            for (const auto& point : local_points) {
                glm::vec2 sp;
                if (ESPMath::WorldToScreen(entityContext.position + point, camera, screenWidth, screenHeight, sp)) {
                    screen_points.push_back(ImVec2(sp.x, sp.y));
                }
                else { projection_ok = false; screen_points.clear(); return; }
            }
            };
        project_ring(localRingXY, screenRingXY);
        project_ring(localRingXZ, screenRingXZ);
        project_ring(localRingYZ, screenRingYZ);

        // --- Draw the 3D sphere ---
        if (projection_ok) {
            // Define the single, final color for all rings.
            unsigned int masterAlpha = (fadedEntityColor >> 24) & 0xFF;
            unsigned int finalLODAlpha = static_cast<unsigned int>(masterAlpha * gyroscopeAlpha);
            ImU32 finalColor = (fadedEntityColor & 0x00FFFFFF) | (finalLODAlpha << 24);

            // Draw all three rings with the same bright color and thickness.
            if (!screenRingXY.empty()) drawList->AddPolyline(screenRingXY.data(), static_cast<int>(screenRingXY.size()), finalColor, false, finalLineThickness);
            if (!screenRingXZ.empty()) drawList->AddPolyline(screenRingXZ.data(), static_cast<int>(screenRingXZ.size()), finalColor, false, finalLineThickness);
            if (!screenRingYZ.empty()) drawList->AddPolyline(screenRingYZ.data(), static_cast<int>(screenRingYZ.size()), finalColor, false, finalLineThickness);
        }
    }
}

void ESPShapeRenderer::RenderGadgetCircle(ImDrawList* drawList, const glm::vec2& screenPos, float radius, unsigned int color, float thickness) {
    drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), radius, color, 0, thickness);
}

void ESPShapeRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                        unsigned int color, float thickness) {
    // Main box
    drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, thickness);

    // Corner indicators for better visibility, scaled with thickness
    const float cornerSize = thickness * RenderingLayout::BOX_CORNER_SIZE_MULTIPLIER;

    // Top-left corner
    drawList->AddLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x + cornerSize, boxMin.y), color, thickness);
    drawList->AddLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x, boxMin.y + cornerSize), color, thickness);

    // Top-right corner
    drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x - cornerSize, boxMin.y), color, thickness);
    drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x, boxMin.y + cornerSize), color, thickness);

    // Bottom-left corner
    drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x + cornerSize, boxMax.y), color, thickness);
    drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x, boxMax.y - cornerSize), color, thickness);

    // Bottom-right corner
    drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x - cornerSize, boxMax.y), color, thickness);
    drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x, boxMax.y - cornerSize), color, thickness);
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
