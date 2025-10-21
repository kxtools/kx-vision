#include "ESPTextRenderer.h"
#include "TextRenderer.h"
#include "../Utils/TextElementFactory.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

void ESPTextRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos,
                                      const std::string& playerName, unsigned int entityColor, float fontSize) {
    if (playerName.empty()) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;
    
    // Use factory to create styled text element
    TextElement element = TextElementFactory::CreatePlayerName(playerName, feetPos, entityColor, fadeAlpha, fontSize);
    
    // Render the element using static method
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderDistanceTextAt(ImDrawList* drawList, const glm::vec2& position, float distance, float fadeAlpha, float fontSize) {
    TextElement element = TextElementFactory::CreateDistanceTextAt(distance, position, fadeAlpha, fontSize);
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax,
                                       const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize) {
    if (details.empty()) return;

    // Calculate anchor position (below the box)
    glm::vec2 anchorPos(center.x, boxMax.y);
    
    // Use factory to create details text element
    TextElement element = TextElementFactory::CreateDetailsText(details, anchorPos, fadeAlpha, fontSize);
    
    // Render the element using static method
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
                                       const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize) {
    if (summary.empty()) return;

    // Use factory to create gear summary text element
    TextElement element = TextElementFactory::CreateGearSummary(summary, feetPos, fadeAlpha, fontSize);
    
    // Render the element using static method
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderDominantStats(ImDrawList* drawList, const glm::vec2& feetPos,
                                         const std::vector<DominantStat>& stats,
                                         Game::ItemRarity topRarity,
                                         float fadeAlpha, float fontSize) {
    if (stats.empty()) return;

    // Use factory to create dominant stats text element
    TextElement element = TextElementFactory::CreateDominantStats(stats, topRarity, feetPos, fadeAlpha, fontSize);
    
    // Render the element using static method
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderPlayerNameAt(ImDrawList* drawList, const glm::vec2& position, 
                                      const std::string& playerName, unsigned int entityColor, float fontSize) {
    if (playerName.empty()) return;

    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;
    
    TextElement element = TextElementFactory::CreatePlayerNameAt(playerName, position, entityColor, fadeAlpha, fontSize);
    
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderDetailsTextAt(ImDrawList* drawList, const glm::vec2& position,
                                       const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize) {
    if (details.empty()) return;

    TextElement element = TextElementFactory::CreateDetailsTextAt(details, position, fadeAlpha, fontSize);
    
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderGearSummaryAt(ImDrawList* drawList, const glm::vec2& position,
                                       const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize) {
    if (summary.empty()) return;

    TextElement element = TextElementFactory::CreateGearSummaryAt(summary, position, fadeAlpha, fontSize);
    
    TextRenderer::Render(drawList, element);
}

void ESPTextRenderer::RenderDominantStatsAt(ImDrawList* drawList, const glm::vec2& position,
                                         const std::vector<DominantStat>& stats,
                                         Game::ItemRarity topRarity,
                                         float fadeAlpha, float fontSize) {
    if (stats.empty()) return;

    TextElement element = TextElementFactory::CreateDominantStatsAt(stats, topRarity, position, fadeAlpha, fontSize);
    
    TextRenderer::Render(drawList, element);
}

} // namespace kx
