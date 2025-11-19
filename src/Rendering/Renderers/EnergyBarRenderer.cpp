#include "EnergyBarRenderer.h"

#include "ShapeRenderer.h"
#include "../../Core/Settings.h"
#include "../../../libs/ImGui/imgui.h"
#include "Shared/ColorConstants.h"
#include "Shared/LayoutConstants.h"

namespace kx {

    void EnergyBarRenderer::Render(
        const Settings& settings,
        ImDrawList* drawList,
        const glm::vec2& barTopLeftPosition,
        float energyPercent,
        float fadeAlpha,
        float barWidth,
        float barHeight) 
    {
        if (energyPercent < 0.0f || energyPercent > 1.0f) return;

        ImVec2 barMin(barTopLeftPosition.x, barTopLeftPosition.y);
        ImVec2 barMax(barTopLeftPosition.x + barWidth, barTopLeftPosition.y + barHeight);

        unsigned int bgAlpha = static_cast<unsigned int>(
            RenderingLayout::STANDALONE_HEALTH_BAR_BG_ALPHA * fadeAlpha * settings.appearance.globalOpacity + 0.5f
        );
        bgAlpha = (bgAlpha > 255) ? 255 : bgAlpha;

        drawList->AddRectFilled(barMin, barMax,
            IM_COL32(0, 0, 0, bgAlpha),
            RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);

        float fillWidth = barWidth * energyPercent;
        ImVec2 eMin(barMin.x, barMin.y);
        ImVec2 eMax(barMin.x + fillWidth, barMax.y);

        ImU32 energyColor = ESPColors::ENERGY_BAR;
        float colorA = ((energyColor >> 24) & 0xFF) / 255.0f;
        
        ImU32 finalColor = ShapeRenderer::ApplyAlphaToColor(
            energyColor, 
            colorA * fadeAlpha * settings.appearance.globalOpacity
        );

        drawList->AddRectFilled(eMin, eMax, finalColor, RenderingLayout::STANDALONE_HEALTH_BAR_BG_ROUNDING);
    }
}

