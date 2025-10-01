#include "ESPHealthBarRenderer.h"
#include "../Utils/ESPConstants.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

static ImVec4 ImLerp(const ImVec4& a, const ImVec4& b, float t) {
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

void ESPHealthBarRenderer::RenderAttachedHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                                   float healthPercent, float fadeAlpha) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    const float barWidth = RenderingLayout::ATTACHED_HEALTH_BAR_WIDTH;
    const float barHeight = boxMax.y - boxMin.y;

    ImVec2 barMin(boxMin.x - barWidth - RenderingLayout::ATTACHED_HEALTH_BAR_SPACING, boxMin.y);
    ImVec2 barMax(boxMin.x - RenderingLayout::ATTACHED_HEALTH_BAR_SPACING, boxMax.y);

    // Background with fade alpha
    unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::ATTACHED_HEALTH_BAR_BG_ALPHA * fadeAlpha);
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
    unsigned int borderAlpha = static_cast<unsigned int>(RenderingLayout::ATTACHED_HEALTH_BAR_BORDER_ALPHA * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32(255, 255, 255, borderAlpha));
}

void ESPHealthBarRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos,
                                                     float healthPercent, unsigned int entityColor,
                                                     float barWidth, float barHeight) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Position the health bar below the entity center
    const float yOffset = RenderingLayout::STANDALONE_HEALTH_BAR_Y_OFFSET;
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);

    // 1. Render the background with a dark, semi-opaque gray to provide neutral contrast.
    unsigned int bgAlpha = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha);
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
    unsigned int healthAlpha = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_HEALTH_ALPHA * fadeAlpha);
    unsigned int finalHealthColor = ImGui::ColorConvertFloat4ToU32(ImVec4(finalColorVec.x, finalColorVec.y, finalColorVec.z, static_cast<float>(healthAlpha) / 255.0f));

    // 5. Draw the final health fill.
    float healthWidth = barWidth * healthPercent;
    drawList->AddRectFilled(barMin, ImVec2(barMin.x + healthWidth, barMax.y), finalHealthColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

    // 6. Add a subtle black border to frame the bar and improve definition.
    unsigned int borderAlpha = static_cast<unsigned int>(RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ALPHA * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32(0, 0, 0, borderAlpha), RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_ROUNDING, 0, RenderingLayout::STANDALONE_HEALTH_BAR_BORDER_THICKNESS);
}

} // namespace kx
