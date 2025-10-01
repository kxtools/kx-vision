#include "ESPShapeRenderer.h"
#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

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

} // namespace kx
