#include "ESPTextRenderer.h"
#include "../Text/TextRenderer.h"
#include "../Text/TextElementFactory.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

void ESPTextRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos,
                                      const std::string& playerName, unsigned int entityColor, float fontSize) {
    if (playerName.empty()) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;
    
    // Use factory to create styled text element
    TextElement element = TextElementFactory::CreatePlayerName(playerName, feetPos, entityColor, fadeAlpha);
    
    // Create renderer and render the element
    TextRenderer renderer(drawList);
    renderer.Render(element);
}

void ESPTextRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin,
                                        float distance, float fadeAlpha, float fontSize) {
    // Calculate anchor position (above the box)
    glm::vec2 anchorPos(center.x, boxMin.y);
    
    // Use factory to create distance text element
    TextElement element = TextElementFactory::CreateDistanceText(distance, anchorPos, fadeAlpha);
    
    // Create renderer and render the element
    TextRenderer renderer(drawList);
    renderer.Render(element);
}

void ESPTextRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax,
                                       const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize) {
    if (details.empty()) return;

    // Calculate anchor position (below the box)
    glm::vec2 anchorPos(center.x, boxMax.y);
    
    // Use factory to create details text element
    TextElement element = TextElementFactory::CreateDetailsText(details, anchorPos, fadeAlpha);
    
    // Create renderer and render the element
    TextRenderer renderer(drawList);
    renderer.Render(element);
}

void ESPTextRenderer::RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
                                       const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize) {
    if (summary.empty()) return;

    // Use factory to create gear summary text element
    TextElement element = TextElementFactory::CreateGearSummary(summary, feetPos, fadeAlpha);
    
    // Create renderer and render the element
    TextRenderer renderer(drawList);
    renderer.Render(element);
}

void ESPTextRenderer::RenderDominantStats(ImDrawList* drawList, const glm::vec2& feetPos,
                                         const std::vector<DominantStat>& stats, float fadeAlpha, float fontSize) {
    if (stats.empty()) return;

    // Use factory to create dominant stats text element
    TextElement element = TextElementFactory::CreateDominantStats(stats, feetPos, fadeAlpha);
    
    // Create renderer and render the element
    TextRenderer renderer(drawList);
    renderer.Render(element);
}

} // namespace kx
