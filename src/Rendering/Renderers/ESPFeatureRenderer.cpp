#include "ESPFeatureRenderer.h"
#include "ESPHealthBarRenderer.h"
#include "ESPShapeRenderer.h"
#include "ESPTextRenderer.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

// ESPFeatureRenderer now acts as a facade, delegating to specialized renderers
// This maintains backwards compatibility while improving code organization

void ESPFeatureRenderer::RenderAttachedHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                                 float healthPercent, float fadeAlpha) {
    ESPHealthBarRenderer::RenderAttachedHealthBar(drawList, boxMin, boxMax, healthPercent, fadeAlpha);
}

void ESPFeatureRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                                   float healthPercent, unsigned int entityColor,
                                                   float barWidth, float barHeight) {
    ESPHealthBarRenderer::RenderStandaloneHealthBar(drawList, centerPos, healthPercent, entityColor, barWidth, barHeight);
}

void ESPFeatureRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos,
                                          const std::string& playerName, unsigned int entityColor, float fontSize) {
    ESPTextRenderer::RenderPlayerName(drawList, feetPos, playerName, entityColor, fontSize);
}

void ESPFeatureRenderer::RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
                                           const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize) {
    ESPTextRenderer::RenderGearSummary(drawList, feetPos, summary, fadeAlpha, fontSize);
}

void ESPFeatureRenderer::RenderDominantStats(ImDrawList* drawList, const glm::vec2& feetPos,
                                             const std::vector<DominantStat>& stats, float fadeAlpha, float fontSize) {
    ESPTextRenderer::RenderDominantStats(drawList, feetPos, stats, fadeAlpha, fontSize);
}

void ESPFeatureRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                           unsigned int color, float thickness) {
    ESPShapeRenderer::RenderBoundingBox(drawList, boxMin, boxMax, color, thickness);
}

void ESPFeatureRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin,
                                            float distance, float fadeAlpha, float fontSize) {
    ESPTextRenderer::RenderDistanceText(drawList, center, boxMin, distance, fadeAlpha, fontSize);
}

void ESPFeatureRenderer::RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos,
                                          unsigned int color, float radius) {
    ESPShapeRenderer::RenderColoredDot(drawList, feetPos, color, radius);
}

void ESPFeatureRenderer::RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos,
                                               float fadeAlpha, float radius) {
    ESPShapeRenderer::RenderNaturalWhiteDot(drawList, feetPos, fadeAlpha, radius);
}

void ESPFeatureRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax,
                                           const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize) {
    ESPTextRenderer::RenderDetailsText(drawList, center, boxMax, details, fadeAlpha, fontSize);
}

unsigned int ESPFeatureRenderer::ApplyAlphaToColor(unsigned int color, float alpha) {
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