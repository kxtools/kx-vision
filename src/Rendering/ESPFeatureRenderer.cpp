#include "ESPFeatureRenderer.h"
#include "../../libs/ImGui/imgui.h"
#include <algorithm>

#include "ESPStyling.h"

namespace kx {

static ImVec4 ImLerp(const ImVec4& a, const ImVec4& b, float t) {
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

void ESPFeatureRenderer::RenderAttachedHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, 
                                                 float healthPercent, float fadeAlpha) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    const float barWidth = 4.0f;
    const float barHeight = boxMax.y - boxMin.y;
    
    ImVec2 barMin(boxMin.x - barWidth - 2.0f, boxMin.y);
    ImVec2 barMax(boxMin.x - 2.0f, boxMax.y);
    
    // Background with fade alpha
    unsigned int bgAlpha = static_cast<unsigned int>(150 * fadeAlpha);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha));
    
    // Health bar - fill from bottom to top with fade alpha
    ImVec2 healthBarMin(barMin.x, barMax.y - (barHeight * healthPercent));
    ImVec2 healthBarMax(barMax.x, barMax.y);
    unsigned int healthAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    unsigned int healthColor = IM_COL32(
        static_cast<int>(255 * (1.0f - healthPercent)),
        static_cast<int>(255 * healthPercent),
        0, healthAlpha
    );
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor);
    
    // Border with fade alpha
    unsigned int borderAlpha = static_cast<unsigned int>(100 * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32(255, 255, 255, borderAlpha));
}

void ESPFeatureRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
    float healthPercent, unsigned int entityColor, float barWidth, float barHeight) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Position the health bar below the entity center
    const float yOffset = 15.0f;
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);

    // 1. Render the background with a dark, semi-opaque gray to provide neutral contrast.
    unsigned int bgAlpha = static_cast<unsigned int>(180 * fadeAlpha);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(50, 50, 50, bgAlpha), 1.0f);

    // 2. Define our bright color keyframes for the gradient.
    const ImVec4 VIBRANT_GREEN = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
    const ImVec4 VIBRANT_YELLOW = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
    const ImVec4 VIBRANT_ORANGE = ImVec4(1.0f, 0.55f, 0.2f, 1.0f);
    const ImVec4 CRITICAL_RED = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);

    // 3. Calculate the health color using a multi-stage linear interpolation (lerp).
    ImVec4 finalColorVec;
    if (healthPercent > 0.5f) {
        float t = (healthPercent - 0.5f) / 0.5f;
        finalColorVec = ImLerp(VIBRANT_YELLOW, VIBRANT_GREEN, t);
    }
    else if (healthPercent > 0.25f) {
        float t = (healthPercent - 0.25f) / 0.25f;
        finalColorVec = ImLerp(VIBRANT_ORANGE, VIBRANT_YELLOW, t);
    }
    else {
        float t = healthPercent / 0.25f;
        finalColorVec = ImLerp(CRITICAL_RED, VIBRANT_ORANGE, t);
    }

    // 4. Apply the distance fade alpha to the final calculated color.
    // Lower the base opacity from 255 (100%) to 220 (~85%) for a more natural feel.
    unsigned int healthAlpha = static_cast<unsigned int>(220 * fadeAlpha);
    unsigned int finalHealthColor = ImGui::ColorConvertFloat4ToU32(ImVec4(finalColorVec.x, finalColorVec.y, finalColorVec.z, static_cast<float>(healthAlpha) / 255.0f));

    // 5. Draw the final health fill.
    float healthWidth = barWidth * healthPercent;
    drawList->AddRectFilled(barMin, ImVec2(barMin.x + healthWidth, barMax.y), finalHealthColor, 1.0f);

    // 6. Add a subtle black border to frame the bar and improve definition.
    unsigned int borderAlpha = static_cast<unsigned int>(100 * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32(0, 0, 0, borderAlpha), 1.0f, 0, 1.0f);
}

void ESPFeatureRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos, 
                                         const std::string& playerName, unsigned int entityColor, float fontSize) {
    if (playerName.empty()) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    ImFont* font = ImGui::GetFont();
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, playerName.c_str());
    
    // Position name just below the feet position (below health bar area)
    const float nameOffset = 25.0f; // Below feet with more padding to avoid health bar overlap
    ImVec2 textPos(feetPos.x - textSize.x / 2, feetPos.y + nameOffset);
    
    // Extract RGB from entity color for a natural look
    int r = (entityColor >> 16) & 0xFF;
    int g = (entityColor >> 8) & 0xFF;
    int b = entityColor & 0xFF;
    
    // Subtle background with rounded corners (like game UI) and distance fade
    ImVec2 bgMin(textPos.x - 4, textPos.y - 2);
    ImVec2 bgMax(textPos.x + textSize.x + 4, textPos.y + textSize.y + 2);
    unsigned int bgAlpha = static_cast<unsigned int>(100 * fadeAlpha);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), 3.0f);
    
    // Subtle border using entity color with distance fade
    unsigned int borderAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddRect(bgMin, bgMax, IM_COL32(r, g, b, borderAlpha), 3.0f, 0, 1.0f);
    
    // Player name text in a clean, readable color with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(220 * fadeAlpha);
    drawList->AddText(font, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), playerName.c_str()); // Shadow
    drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, textAlpha), playerName.c_str()); // Main text
}

void ESPFeatureRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, 
                                          unsigned int color, float thickness) {
    // Main box
    drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, thickness);
    
    // Corner indicators for better visibility, scaled with thickness
    const float cornerSize = thickness * 4.0f;
    
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

void ESPFeatureRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, 
                                           float distance, float fadeAlpha, float fontSize) {
    char distText[32];
    snprintf(distText, sizeof(distText), "%.1fm", distance);
    
    ImFont* font = ImGui::GetFont();
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, distText);
    ImVec2 textPos(center.x - textSize.x / 2, boxMin.y - textSize.y - 5);
    
    // Background with distance fade - REDUCED OPACITY
    unsigned int bgAlpha = static_cast<unsigned int>(100 * fadeAlpha); // Before: 150
    drawList->AddRectFilled(ImVec2(textPos.x - 2, textPos.y - 1),
        ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1),
        IM_COL32(0, 0, 0, bgAlpha), 2.0f);

    // Text with shadow and distance fade - REDUCED OPACITY
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha); // Before: 255
    unsigned int textAlpha = static_cast<unsigned int>(220 * fadeAlpha);   // Before: 255
    drawList->AddText(font, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), distText);
    drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, textAlpha), distText);
}

void ESPFeatureRenderer::RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos, unsigned int color, float radius) {
    // Extract fade alpha from the color parameter
    float fadeAlpha = ((color >> 24) & 0xFF) / 255.0f;
    
    // Small, minimalistic dot with subtle outline for visibility
    // Dark outline with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), radius, IM_COL32(0, 0, 0, shadowAlpha));
    // Main dot using entity color (already has faded alpha)
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), radius * 0.8f, color);
}

void ESPFeatureRenderer::RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos, float fadeAlpha, float radius) {
    ImVec2 pos(feetPos.x, feetPos.y);
    
    // Shadow with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(pos.x + 1, pos.y + 1), radius, IM_COL32(0, 0, 0, shadowAlpha));
    
    // Dot with distance fade
    unsigned int dotAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    drawList->AddCircleFilled(pos, radius * 0.8f, IM_COL32(255, 255, 255, dotAlpha));
}

void ESPFeatureRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax,
    const std::vector<ColoredDetail>& details, float fadeAlpha, float fontSize) {
    if (details.empty()) return;

    float textY = boxMax.y + 5.0f;
    ImFont* font = ImGui::GetFont();

    for (const auto& detail : details) {
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, detail.text.c_str());
        ImVec2 textPos(center.x - textSize.x / 2, textY);

        // Background with distance fade - REDUCED OPACITY
        unsigned int bgAlpha = static_cast<unsigned int>(100 * fadeAlpha); // Before: 160
        drawList->AddRectFilled(ImVec2(textPos.x - 3, textPos.y - 1),
            ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 1),
            IM_COL32(0, 0, 0, bgAlpha), 1.0f);

        // Text with shadow and distance fade - REDUCED SHADOW OPACITY
        unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha); // Before: 200

        // Extract the original alpha from the detail color and combine it with the fade alpha
        int originalAlpha = (detail.color >> 24) & 0xFF;
        unsigned int textAlpha = static_cast<unsigned int>(originalAlpha * fadeAlpha);

        // Re-create the final text color with the new combined alpha
        ImU32 finalTextColor = (detail.color & 0x00FFFFFF) | (textAlpha << 24);

        drawList->AddText(font, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), detail.text.c_str());
        drawList->AddText(font, fontSize, textPos, finalTextColor, detail.text.c_str());

        textY += textSize.y + 3;
    }
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

void ESPFeatureRenderer::RenderGearSummary(ImDrawList* drawList, const glm::vec2& feetPos,
    const std::vector<CompactStatInfo>& summary, float fadeAlpha, float fontSize) {
    if (summary.empty()) return;

    ImFont* font = ImGui::GetFont();

    // --- Part 1: Calculate total width for centering ---
    float totalWidth = 0.0f;
    const std::string prefix = "Stats: ";
    totalWidth += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, prefix.c_str()).x;

    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        std::string segment = std::to_string(info.count) + "x " + info.statName;
        totalWidth += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, segment.c_str()).x;
        if (i < summary.size() - 1) {
            totalWidth += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, ", ").x;
        }
    }

    // --- Part 2: Render the multi-colored line ---
    const float summaryOffset = 45.0f;
    ImVec2 currentPos(feetPos.x - totalWidth / 2.0f, feetPos.y + summaryOffset);

    // Fade alphas
    unsigned int bgAlpha = static_cast<unsigned int>(80 * fadeAlpha);
    unsigned int shadowAlpha = static_cast<unsigned int>(160 * fadeAlpha);
    unsigned int defaultTextAlpha = static_cast<unsigned int>(200 * fadeAlpha);

    // Background
    ImVec2 bgMin(currentPos.x - 4, currentPos.y - 2);
    ImVec2 bgMax(currentPos.x + totalWidth + 4, currentPos.y + font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, " ").y + 2);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), 3.0f);

    // Render "Stats: " prefix in default color
    drawList->AddText(font, fontSize, ImVec2(currentPos.x + 1, currentPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), prefix.c_str());
    drawList->AddText(font, fontSize, currentPos, IM_COL32(200, 210, 255, defaultTextAlpha), prefix.c_str());
    currentPos.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, prefix.c_str()).x;

    // Render each colored segment
    for (size_t i = 0; i < summary.size(); ++i) {
        const auto& info = summary[i];
        std::string segment = std::to_string(info.count) + "x " + info.statName;
        ImU32 rarityColor = ESPHelpers::GetRarityColor(info.highestRarity);

        // Apply fade to the rarity color
        int r = (rarityColor >> 0) & 0xFF;
        int g = (rarityColor >> 8) & 0xFF;
        int b = (rarityColor >> 16) & 0xFF;
        rarityColor = IM_COL32(r, g, b, defaultTextAlpha);

        drawList->AddText(font, fontSize, ImVec2(currentPos.x + 1, currentPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), segment.c_str());
        drawList->AddText(font, fontSize, currentPos, rarityColor, segment.c_str());
        currentPos.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, segment.c_str()).x;

        // Render separator
        if (i < summary.size() - 1) {
            const std::string separator = ", ";
            drawList->AddText(font, fontSize, ImVec2(currentPos.x + 1, currentPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), separator.c_str());
            drawList->AddText(font, fontSize, currentPos, IM_COL32(200, 210, 255, defaultTextAlpha), separator.c_str());
            currentPos.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, separator.c_str()).x;
        }
    }
}

void ESPFeatureRenderer::RenderDominantStats(ImDrawList* drawList, const glm::vec2& feetPos,
    const std::vector<DominantStat>& stats, float fadeAlpha, float fontSize) {
    if (stats.empty()) return;

    ImFont* font = ImGui::GetFont();

    // 1. Build the display string
    std::string summaryText = "[";
    for (size_t i = 0; i < stats.size(); ++i) {
        summaryText += stats[i].name;
        if (i < stats.size() - 1) {
            summaryText += " | ";
        }
    }
    summaryText += "]";

    // 2. Calculate width and position
    float totalWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, summaryText.c_str()).x;
    const float summaryOffset = 45.0f; // Use same offset as the other compact view for consistency
    ImVec2 textPos(feetPos.x - totalWidth / 2.0f, feetPos.y + summaryOffset);

    // 3. Render
    unsigned int bgAlpha = static_cast<unsigned int>(80 * fadeAlpha);
    unsigned int shadowAlpha = static_cast<unsigned int>(160 * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(200 * fadeAlpha);

    ImVec2 bgMin(textPos.x - 4, textPos.y - 2);
    ImVec2 bgMax(textPos.x + totalWidth + 4, textPos.y + font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, " ").y + 2);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), 3.0f);

    drawList->AddText(font, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), summaryText.c_str());
    drawList->AddText(font, fontSize, textPos, IM_COL32(200, 210, 255, textAlpha), summaryText.c_str());
}

} // namespace kx